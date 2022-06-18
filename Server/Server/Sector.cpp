#include "stdafx.h"
#include "Sector.h"

#include "IocpBase.h"
#include "Protocol.h"
#include "Session.h"
#include "SectorManager.h"
#include "NPC.h"
#include "Random.h"
#include "Timer.h"

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

	Sector::Sector(const std::vector<std::vector<Tile>>& tileMap, int sectorNum)
		: mTileMap{ tileMap }
		, mSectorNum{ sectorNum }
	{
		auto npcPerSector = NUM_NPC / (gSectorsPerLine * gSectorsPerLine);
		auto quater = npcPerSector / 4;

		for (int i = 0; i < quater; ++i)
		{
			auto npc = new NPC;
			auto freeID = GetFreeNpcID();
			gClients[freeID] = npc;
			npc->SetID(freeID);
			npc->SetName("»ÔÃæÀÌ");
			npc->SetLevel(mSectorNum / gSectorsPerLine + 1);
			npc->SetMaxHP(mSectorNum / gSectorsPerLine + 3);
			npc->SetCurrentHP(npc->GetMaxHP());
			npc->SetRace(Race::Enemy1);
			npc->SetHostile(true);
			npc->SetBehaviorType(NpcBehaviorType::PEACE);
			npc->SetMoveType(NpcMoveType::FIXED);
			auto [x, y] = getAvailablePos(0);
			npc->SetPos(x, y);
			npc->SetAttackPower(npc->GetLevel());
			auto level = npc->GetLevel();
			auto exp = level * level
				* static_cast<int>(npc->GetBehaviorType())
				* static_cast<int>(npc->GetMoveType());
			npc->SetExp(exp);
			mActorIds.insert(freeID);
		}

		for (int i = quater; i < quater * 2; ++i)
		{
			auto npc = new NPC;
			auto freeID = GetFreeNpcID();
			gClients[freeID] = npc;
			npc->SetID(freeID);
			npc->SetName("ÆÒÅÒ");
			npc->SetLevel(mSectorNum / gSectorsPerLine + 2);
			npc->SetMaxHP(mSectorNum / gSectorsPerLine + 5);
			npc->SetCurrentHP(npc->GetMaxHP());
			npc->SetRace(Race::Enemy2);
			npc->SetHostile(true);
			npc->SetBehaviorType(NpcBehaviorType::PEACE);
			npc->SetMoveType(NpcMoveType::FIXED);
			auto [x, y] = getAvailablePos(1);
			npc->SetPos(x, y);
			npc->SetAttackPower(npc->GetLevel());
			auto level = npc->GetLevel();
			auto exp = level * level
				* static_cast<int>(npc->GetBehaviorType())
				* static_cast<int>(npc->GetMoveType());
			npc->SetExp(exp);
			mActorIds.insert(freeID);
		}

		for (int i = quater * 2; i < quater * 3; ++i)
		{
			auto npc = new NPC;
			auto freeID = GetFreeNpcID();
			gClients[freeID] = npc;
			npc->SetID(freeID);
			npc->SetName("±ÙÀ°¸ó");
			npc->SetLevel(mSectorNum / gSectorsPerLine + 3);
			npc->SetMaxHP(mSectorNum / gSectorsPerLine + 7);
			npc->SetCurrentHP(npc->GetMaxHP());
			npc->SetRace(Race::Enemy3);
			npc->SetHostile(true);
			npc->SetBehaviorType(NpcBehaviorType::PEACE);
			npc->SetMoveType(NpcMoveType::FIXED);
			auto [x, y] = getAvailablePos(2);
			npc->SetPos(x, y);
			npc->SetAttackPower(npc->GetLevel());
			auto level = npc->GetLevel();
			auto exp = level * level
				* static_cast<int>(npc->GetBehaviorType())
				* static_cast<int>(npc->GetMoveType());
			npc->SetExp(exp);
			mActorIds.insert(freeID);
		}

		for (int i = quater * 3; i < quater * 3 + 1; ++i)
		{
			auto npc = new NPC;
			auto freeID = GetFreeNpcID();
			gClients[freeID] = npc;
			npc->SetID(freeID);
			npc->SetName("ÇÇÄ«Ãò");
			npc->SetLevel(mSectorNum / gSectorsPerLine + 4);
			npc->SetMaxHP(mSectorNum / gSectorsPerLine + 10);
			npc->SetCurrentHP(npc->GetMaxHP());
			npc->SetRace(Race::Enemy4);
			npc->SetHostile(true);
			npc->SetBehaviorType(NpcBehaviorType::PEACE);
			npc->SetMoveType(NpcMoveType::FIXED);
			auto [x, y] = getAvailablePos(3);
			npc->SetPos(x, y);
			npc->SetAttackPower(npc->GetLevel());
			auto level = npc->GetLevel();
			auto exp = level * level
				* static_cast<int>(npc->GetBehaviorType())
				* static_cast<int>(npc->GetMoveType());
			npc->SetExp(exp);
			mActorIds.insert(freeID);
		}
	}

	void Sector::AddActor(Actor* target)
	{
		auto targetID = target->GetID();

		{
			ReadLockGuard guard = { mLock };
			if (0 != mActorIds.count(targetID))
			{
				MK_SLOG("AddActor() : Actor[{0}] is already in sector[{1}]", targetID,
					mSectorNum);
				return;
			}
		}

		std::unordered_set<id_type> actorIds;
		{
			WriteLockGuard guard = { mLock };
			mActorIds.insert(targetID);
			actorIds = mActorIds;
		}

		auto targetPos = target->GetPos();

		for (auto actorID : actorIds)
		{
			if (actorID == targetID) continue;

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
				MK_SLOG("RemoveActor() : Actor[{0}] is not in sector[{1}]", targetID,
					mSectorNum);
				return;
			}
		}

		std::unordered_set<id_type> viewList;
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
			MK_SLOG("MoveActor : Actor[{0}]: is not in sector[{1}]", targetID,
				mSectorNum);
			AddActor(target);
			return;
		}
		mLock.ReadUnlock();

		auto [x, y] = target->GetPos();
		switch (direction)
		{
		case UP:
			if (y > 0 && NOT isSolid(y - 1, x)) y--;
			break;
		case DOWN:
			if (y < W_HEIGHT - 1 && NOT isSolid(y + 1, x)) y++;
			break;
		case LEFT:
			if (x > 0 && NOT isSolid(y, x - 1)) x--;
			break;
		case RIGHT:
			if (x < W_WIDTH - 1 && NOT isSolid(y, x + 1)) x++;
			break;
		default:
			MK_ASSERT(false);
			break;
		}

		target->SetPos(x, y);
		static_cast<Session*>(target)->SendMovePacket(targetID, clientTime);

		bool bOut = isOutOfBound(x, y);
		if (bOut)
		{
			SectorManager::ChangeSector(target, mSectorNum);
			return;
		}

		// ±âÁ¸ ViewList º¹»ç
		std::unordered_set<id_type> oldList;
		{
			ReadLockGuard guard = { target->ViewLock };
			oldList = target->ViewList;
		}

		// NearList »ý¼º
		std::unordered_set<id_type> nearList;
		{
			ReadLockGuard guard = { mLock };
			for (auto actorID : mActorIds)
			{
				if (actorID == targetID)
				{
					continue;
				}

				bool bInRange = isInView({ x, y }, gClients[actorID]->GetPos());

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
		std::unordered_set<id_type> nearList;
		{
			ReadLockGuard guard = { target->ViewLock };
			nearList = target->ViewList;
		}

		static_cast<Session*>(target)->SendChatPacket(target->GetID(),
			chatType, chat);

		for (auto actorID : nearList)
		{
			if (actorID < MAX_USER)
			{
				static_cast<Session*>(gClients[actorID])->SendChatPacket(target->GetID(),
					chatType, chat);
			}
		}
	}

	void Sector::DoAttack(Actor* hitter)
	{
		using namespace std::chrono;

		Timer::AddEvent(TimerEventType::EV_RESET_ATTACK, hitter->GetID(),
			system_clock::now() + 1s);

		{
			WriteLockGuard guard = { hitter->ActorLock };
			hitter->SetAttack(false);
		}

		std::unordered_set<id_type> viewList;
		{
			ReadLockGuard guard = { hitter->ViewLock };
			viewList = hitter->ViewList;
		}

		auto [x, y] = hitter->GetPos();

		for (auto actorID : viewList)
		{
			if (actorID < MAX_USER) continue;

			bool bInRange = isInAttackRange({ x, y }, gClients[actorID]->GetPos());

			if (bInRange)
			{
				auto victim = gClients[actorID];

				auto attackPower = hitter->GetAttackPower();
				int victimHP = 0;
				{
					ReadLockGuard guard = { victim->ActorLock };
					victimHP = victim->GetCurrentHP();
				}
				victimHP -= attackPower;

				static_cast<Session*>(hitter)->SendSystemChatDamage(actorID);

				if (victimHP <= 0)
				{
					// TODO : NPC »ç¸Á Ã³¸®
					static_cast<Session*>(hitter)->SendSystemChatExp(actorID);
					
					auto victimExp = victim->GetExp();
					{
						WriteLockGuard guard = { hitter->ActorLock };
						auto currentExp = hitter->GetExp();
						hitter->SetExp(currentExp + victimExp);
					}
					
					auto hitterID = hitter->GetID();
					static_cast<Session*>(hitter)->SendStatChangePacket(hitterID);
					
					RemoveActor(victim);
				}
				else
				{
					{
						WriteLockGuard guard = { victim->ActorLock };
						victim->SetCurrentHP(victimHP);
					}
					SendStatChangeToViewList(victim);
				}
			}
		}
	}

	void Sector::SendStatChangeToViewList(Actor* target)
	{
		std::unordered_set<id_type> viewList;
		{
			ReadLockGuard guard = { target->ViewLock };
			viewList = target->ViewList;
		}

		auto targetID = target->GetID();

		if (targetID < MAX_USER)
		{
			static_cast<Session*>(target)->SendStatChangePacket(targetID);
		}

		for (auto actorID : viewList)
		{
			if (actorID >= MAX_USER) continue;

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

	bool Sector::isInView(const pos_type& aPos, const pos_type& bPos)
	{
		if (bPos.first < aPos.first - 7 ||
			bPos.first > aPos.first + 7 ||
			bPos.second < aPos.second - 7 ||
			bPos.second > aPos.second + 7)
		{
			return false;
		}

		return true;
	}

	bool Sector::isInAttackRange(const pos_type& hitterPos, const pos_type& victimPos)
	{
		if (hitterPos.first + 1 == victimPos.first
			&& hitterPos.second == victimPos.second)
		{
			return true;
		}

		if (hitterPos.first - 1 == victimPos.first
			&& hitterPos.second == victimPos.second)
		{
			return true;
		}

		if (hitterPos.first == victimPos.first
			&& hitterPos.second + 1 == victimPos.second)
		{
			return true;
		}

		if (hitterPos.first == victimPos.first
			&& hitterPos.second - 1 == victimPos.second)
		{
			return true;
		}

		if (hitterPos.first == victimPos.first
			&& hitterPos.second == victimPos.second)
		{
			return true;
		}

		return false;
	}

	Sector::pos_type Sector::getAvailablePos(const int area)
	{
		const POINT leftTop = { (mSectorNum % gSectorsPerLine) * TILE_PER_SECTOR,
			(mSectorNum / gSectorsPerLine) * TILE_PER_SECTOR };

		short x = 0;
		short y = 0;

		switch (area)
		{
		case 0: // 2»çºÐ¸é
		{
			do
			{
				x = Random::RandInt(leftTop.x, leftTop.x + 19);
				y = Random::RandInt(leftTop.y, leftTop.y + 19);
			} while (isSolid(y, x));

			return { x, y };
		}

		case 1: // 1»çºÐ¸é
		{
			do
			{
				x = Random::RandInt(leftTop.x + 20, leftTop.x + 39);
				y = Random::RandInt(leftTop.y, leftTop.y + 19);
			} while (isSolid(y, x));

			return { x, y };
		}

		case 2: // 3»çºÐ¸é
		{
			do
			{
				x = Random::RandInt(leftTop.x, leftTop.x + 19);
				y = Random::RandInt(leftTop.y + 20, leftTop.y + 39);
			} while (isSolid(y, x));

			return { x, y };
		}

		case 3: // 4»çºÐ¸é
		{
			return { static_cast<int>(leftTop.x) + 30,
				static_cast<int>(leftTop.y) + 30 };
		}

		default:
			MK_ASSERT(false);
			return { -1, -1 };
		}
	}
}
