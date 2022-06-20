#include "stdafx.h"
#include "Sector.h"

#include "IocpBase.h"
#include "Protocol.h"
#include "Session.h"
#include "SectorManager.h"
#include "NPC.h"
#include "Random.h"
#include "Timer.h"
#include "AIState.h"

namespace mk
{
	enum
	{
		UP,
		DOWN,
		LEFT,
		RIGHT
	};

	int GetFreeNpcID()
	{
		for (int idx = MAX_USER; idx < MAX_USER + NUM_NPC; ++idx)
		{
			if (NOT gClients[idx])
			{
				return idx;
			}
		}

		return -1;
	}

	bool IsUser(const id_t id)
	{
		return id < MAX_USER;
	}

	vec2 GetUserRegenPos(const short x, const short y)
	{
		return vec2((x / 40) * 40 + 4, (y / 40) * 40 + 4);
	}

	Sector::Sector(const std::vector<std::vector<Tile>>& tileMap, int sectorNum)
		: mTileMap{ tileMap }
		, mSectorNum{ sectorNum }
	{
		auto npcPerSector = NUM_NPC / (gSectorsPerLine * gSectorsPerLine);
		auto quater = npcPerSector / 4;

		for (int i = 0; i < quater; ++i)
		{
			createEnemy(Race::Enemy1, mSectorNum + 1);
		}

		for (int i = quater; i < quater * 2; ++i)
		{
			createEnemy(Race::Enemy2, mSectorNum + 2);
		}

		for (int i = quater * 2; i < quater * 3; ++i)
		{
			createEnemy(Race::Enemy3, mSectorNum + 3);
		}

		for (int i = quater * 3; i < quater * 3 + 1; ++i)
		{
			createEnemy(Race::Enemy4, mSectorNum + 5);
		}
	}

	void Sector::AddActor(Actor* target)
	{
		auto targetID = target->GetID();

		{
			ReadLockGuard guard = { mLock };
			if (0 != mActorIds.count(targetID))
			{
				return;
			}
		}

		if (IsUser(targetID))
		{
			mNumSessions.fetch_add(1);
		}

		std::unordered_set<id_t> actorIds;
		{
			WriteLockGuard guard = { mLock };
			actorIds = mActorIds;
			mActorIds.insert(targetID);
		}

		auto targetPos = target->GetPos();

		for (auto actorID : actorIds)
		{
			auto actor = gClients[actorID];
			auto actorPos = actor->GetPos();
			bool bInView = isInView(targetPos, actorPos);

			if (bInView)
			{
				target->AddToViewList(actorID);
				actor->AddToViewList(targetID);
			}
		}
	}

	void Sector::RemoveActor(Actor* target)
	{
		auto targetID = target->GetID();
		{
			WriteLockGuard guard = { mLock };
			auto cnt = mActorIds.erase(targetID);
			if (0 == cnt)
			{
				return;
			}
		}

		if (IsUser(targetID))
		{
			mNumSessions.fetch_sub(1);
		}

		std::unordered_set<id_t> viewList;
		{
			ReadLockGuard guard = { target->ViewLock };
			viewList = target->ViewList;
		}

		for (auto actorID : viewList)
		{
			target->RemoveFromViewList(actorID);
			gClients[actorID]->RemoveFromViewList(targetID);
		}
	}

	void Sector::MoveActor(Actor* target, const char direction,
		const unsigned int clientTime)
	{
		auto targetID = target->GetID();

		mLock.ReadLock();
		if (0 == mActorIds.count(targetID))
		{
			mLock.ReadUnlock();
			AddActor(target);
			return;
		}
		mLock.ReadUnlock();

		auto targetPos = target->GetPos();
		switch (direction)
		{
		case UP:
			if (targetPos.y > 0 && NOT isSolid(targetPos.y - 1, targetPos.x))
				targetPos.y--;
			break;
		case DOWN:
			if (targetPos.y < W_HEIGHT - 1 && NOT isSolid(targetPos.y + 1, targetPos.x))
				targetPos.y++;
			break;
		case LEFT:
			if (targetPos.x > 0 && NOT isSolid(targetPos.y, targetPos.x - 1))
				targetPos.x--;
			break;
		case RIGHT:
			if (targetPos.x < W_WIDTH - 1 && NOT isSolid(targetPos.y, targetPos.x + 1))
				targetPos.x++;
			break;
		default:
			MK_ASSERT(false);
			break;
		}

		target->SetPos(targetPos);
		static_cast<Session*>(target)->SendMovePacket(targetID, clientTime);

		bool bOut = isOutOfBound(targetPos.x, targetPos.y);
		if (bOut)
		{
			SectorManager::ChangeSector(target, mSectorNum);
			return;
		}

		// ±âÁ¸ ViewList º¹»ç
		std::unordered_set<id_t> oldList;
		{
			ReadLockGuard guard = { target->ViewLock };
			oldList = target->ViewList;
		}

		// NearList »ý¼º
		std::unordered_set<id_t> nearList;
		{
			ReadLockGuard guard = { mLock };
			for (auto actorID : mActorIds)
			{
				if (actorID == targetID)
				{
					continue;
				}

				bool bInRange = isInView(targetPos, gClients[actorID]->GetPos());

				if (bInRange)
				{
					nearList.insert(actorID);
				}
			}
		}

		for (auto actorID : nearList)
		{
			target->AddToViewList(actorID);
			gClients[actorID]->AddToViewList(targetID, true);
		}

		for (auto actorID : oldList)
		{
			if (0 != nearList.count(actorID))
			{
				continue;
			}

			target->RemoveFromViewList(actorID);
			gClients[actorID]->RemoveFromViewList(targetID);
		}
	}

	void Sector::SendChatToViewList(Actor* target, const char chatType, std::string_view chat)
	{
		std::unordered_set<id_t> nearList;
		{
			ReadLockGuard guard = { target->ViewLock };
			nearList = target->ViewList;
		}

		static_cast<Session*>(target)->SendChatPacket(target->GetID(),
			chatType, chat);

		for (auto actorID : nearList)
		{
			if (IsUser(actorID))
			{
				static_cast<Session*>(gClients[actorID])->SendChatPacket(target->GetID(),
					chatType, chat);
			}
		}
	}

	void Sector::DoAttack(Actor* hitter)
	{
		using namespace std::chrono;

		std::unordered_set<id_t> viewList;
		{
			ReadLockGuard guard = { hitter->ViewLock };
			viewList = hitter->ViewList;
		}

		auto hitterPos = hitter->GetPos();

		for (auto actorID : viewList)
		{
			if (IsUser(actorID)) continue;

			auto victim = gClients[actorID];
			auto victimPos = victim->GetPos();
			bool bInRange = isInAttackRange(hitterPos, victimPos);

			if (bInRange)
			{
				auto attackPower = hitter->GetAttackPower();
				int victimHP = 0;
				{
					ReadLockGuard guard = { victim->ActorLock };
					victimHP = victim->GetCurrentHP();
				}
				victimHP -= attackPower;

				auto hitterSession = static_cast<Session*>(hitter);
				hitterSession->SendSystemChatDamage(actorID);

				if (victimHP <= 0)
				{
					auto victimExp = victim->GetExp();
					hitterSession->OnKillEnemy(victimExp);
					hitterSession->SendSystemChatExp(actorID);

					RemoveActor(victim);

					{
						WriteLockGuard guard = { victim->ActorLock };
						victim->SetActive(false);
						static_cast<NPC*>(victim)->SetStateToInitial();
					}

					TimerEvent ev = {};
					ev.EventType = TimerEventType::EV_REGEN_ENEMY;
					ev.ID = actorID;
					ev.ActTime = system_clock::now() + 30s;
					CopyMemory(ev.ExtraData, &mSectorNum, sizeof(mSectorNum));
					Timer::AddEvent(ev);
				}
				else
				{
					{
						WriteLockGuard guard = { victim->ActorLock };
						victim->SetCurrentHP(victimHP);
						auto hitterID = hitter->GetID();
						static_cast<NPC*>(victim)->OnHit(hitterID);
					}
					SendStatChangeToViewList(victim);
				}
			}
		}
	}

	void Sector::SendStatChangeToViewList(Actor* target)
	{
		std::unordered_set<id_t> viewList;
		{
			ReadLockGuard guard = { target->ViewLock };
			viewList = target->ViewList;
		}

		auto targetID = target->GetID();

		if (IsUser(targetID))
		{
			static_cast<Session*>(target)->SendStatChangePacket(targetID);
		}

		for (auto actorID : viewList)
		{
			if (NOT IsUser(actorID)) continue;

			(gClients[actorID]->ViewLock).ReadLock();
			if (0 != gClients[actorID]->ViewList.count(targetID))
			{
				(gClients[actorID]->ViewLock).ReadUnlock();
				static_cast<Session*>(gClients[actorID])->SendStatChangePacket(targetID);
			}
			else
			{
				(gClients[actorID]->ViewLock).ReadUnlock();
			}
		}
	}

	void Sector::RegenEnemy(Actor* actor)
	{
		NPC* enemy = static_cast<NPC*>(actor);
		vec2 newPos = getAvailablePos(enemy->GetRace());

		{
			WriteLockGuard guard = { enemy->ActorLock };
			enemy->SetPos(newPos);
			enemy->SetCurrentHP(enemy->GetMaxHP());
		}

		AddActor(actor);
	}

	void Sector::RegenUser(Actor* actor)
	{
		RemoveActor(actor);

		auto actorPos = actor->GetPos();
		vec2 newPos = GetUserRegenPos(actorPos.x, actorPos.y);

		{
			WriteLockGuard guard = { actor->ActorLock };
			actor->SetPos(newPos);
		}

		AddActor(actor);
	}

	void Sector::SendNpcMoveToViewList(Actor* target)
	{
		// ±âÁ¸ ViewList º¹»ç
		std::unordered_set<id_t> oldList;
		{
			ReadLockGuard guard = { target->ViewLock };
			oldList = target->ViewList;
		}

		auto targetID = target->GetID();
		auto targetPos = target->GetPos();

		// NearList »ý¼º
		std::unordered_set<id_t> nearList;
		{
			ReadLockGuard guard = { mLock };
			for (auto actorID : mActorIds)
			{
				if (actorID == targetID)
				{
					continue;
				}

				bool bInRange = isInView(targetPos, gClients[actorID]->GetPos());

				if (bInRange)
				{
					nearList.insert(actorID);
				}
			}
		}

		for (auto actorID : nearList)
		{
			target->AddToViewList(actorID);
			gClients[actorID]->AddToViewList(targetID, true);
		}

		for (auto actorID : oldList)
		{
			if (0 != nearList.count(actorID))
			{
				continue;
			}

			target->RemoveFromViewList(actorID);
			gClients[actorID]->RemoveFromViewList(targetID);
		}
	}

	bool Sector::isSolid(const short row, const short col)
	{
		return mTileMap[row % TILE_PER_SECTOR][col % TILE_PER_SECTOR].Solidity == 1;
	}

	bool Sector::isOutOfBound(const short x, const short y)
	{
		const POINT leftTop = { (mSectorNum % gSectorsPerLine) * TILE_PER_SECTOR,
			(mSectorNum / gSectorsPerLine) * TILE_PER_SECTOR };

		if (x < leftTop.x)
		{
			return true;
		}

		if (x > leftTop.x + (TILE_PER_SECTOR - 1))
		{
			return true;
		}

		if (y < leftTop.y)
		{
			return true;
		}

		if (y > leftTop.y + (TILE_PER_SECTOR - 1))
		{
			return true;
		}

		return false;
	}

	bool Sector::isInView(const vec2& aPos, const vec2& bPos)
	{
		if (bPos.x < aPos.x - 7 ||
			bPos.x > aPos.x + 7 ||
			bPos.y < aPos.y - 7 ||
			bPos.y > aPos.y + 7)
		{
			return false;
		}

		return true;
	}

	bool Sector::isInAttackRange(const vec2& hitterPos, const vec2& victimPos)
	{
		if (hitterPos.x + 1 == victimPos.x
			&& hitterPos.y == victimPos.y)
		{
			return true;
		}

		if (hitterPos.x - 1 == victimPos.x
			&& hitterPos.y == victimPos.y)
		{
			return true;
		}

		if (hitterPos.x == victimPos.x
			&& hitterPos.y + 1 == victimPos.y)
		{
			return true;
		}

		if (hitterPos.x == victimPos.x
			&& hitterPos.y - 1 == victimPos.y)
		{
			return true;
		}

		if (hitterPos.x == victimPos.x
			&& hitterPos.y == victimPos.y)
		{
			return true;
		}

		return false;
	}

	vec2 Sector::getAvailablePos(const int race)
	{
		const vec2 leftTop = vec2
		{
			static_cast<short>((mSectorNum % gSectorsPerLine) * TILE_PER_SECTOR),
			static_cast<short>((mSectorNum / gSectorsPerLine) * TILE_PER_SECTOR)
		};

		short x = 0;
		short y = 0;

		switch (race)
		{
		case Race::Enemy1:
		{
			do
			{
				x = Random::RandInt(leftTop.x, leftTop.x + 19);
				y = Random::RandInt(leftTop.y, leftTop.y + 19);
			} while (isSolid(y, x));

			return { x, y };
		}

		case Race::Enemy2:
		{
			do
			{
				x = Random::RandInt(leftTop.x + 20, leftTop.x + 39);
				y = Random::RandInt(leftTop.y, leftTop.y + 19);
			} while (isSolid(y, x));

			return { x, y };
		}

		case Race::Enemy3:
		{
			do
			{
				x = Random::RandInt(leftTop.x, leftTop.x + 19);
				y = Random::RandInt(leftTop.y + 20, leftTop.y + 39);
			} while (isSolid(y, x));

			return { x, y };
		}

		case Race::Enemy4:
		{
			return vec2{ static_cast<short>(leftTop.x + 30),
				static_cast<short>(leftTop.y + 30) };
		}

		default:
			MK_ASSERT(false);
			return { -1, -1 };
		}
	}

	void Sector::createEnemy(const int race, const int level)
	{
		lua_getglobal(gLuaVM, "GetNpcInfo");
		lua_pushnumber(gLuaVM, level);
		lua_pcall(gLuaVM, 1, 2, 0);

		int maxHP = static_cast<int>(lua_tonumber(gLuaVM, -2));
		int attackPower = static_cast<int>(lua_tonumber(gLuaVM, -1));
		lua_pop(gLuaVM, 2);

		auto npc = new NPC;
		auto freeID = GetFreeNpcID();
		gClients[freeID] = npc;
		npc->SetID(freeID);
		npc->SetLevel(level);
		npc->SetHostile(true);
		npc->SetMaxHP(maxHP);
		npc->SetCurrentHP(maxHP);
		npc->SetAttackPower(attackPower);
		npc->SetRace(race);
		auto [x, y] = getAvailablePos(race);
		npc->SetPos(x, y);
		
		switch (race)
		{
		case Race::Enemy1:
			npc->SetName("»ÔÃæÀÌ");
			npc->SetBehaviorType(NpcBehaviorType::PEACE);
			npc->SetMoveType(NpcMoveType::FIXED);
			npc->ChangeState(new PeaceState{ npc, false });
			break;

		case Race::Enemy2:
			npc->SetName("ÆÒÅÒ");
			npc->SetBehaviorType(NpcBehaviorType::PEACE);
			npc->SetMoveType(NpcMoveType::ROAMING);
			npc->ChangeState(new RoamingState{ npc, false });
			break;

		case Race::Enemy3:
			npc->SetName("±ÙÀ°¸ó");
			npc->SetBehaviorType(NpcBehaviorType::AGGRO);
			npc->SetMoveType(NpcMoveType::ROAMING);
			npc->ChangeState(new RoamingState{ npc, true });
			break;

		case Race::Enemy4:
			npc->SetName("ÇÇÄ«Ãò");
			npc->SetBehaviorType(NpcBehaviorType::AGGRO);
			npc->SetMoveType(NpcMoveType::FIXED);
			npc->ChangeState(new PeaceState{ npc, true });

#ifdef MK_TEST
			npc->SetAttackPower(100);
#endif
			break;
		}

		auto exp = level * level
			* static_cast<int>(npc->GetBehaviorType())
			* static_cast<int>(npc->GetMoveType());
		npc->SetExp(exp);

		mActorIds.insert(freeID);
	}

}
