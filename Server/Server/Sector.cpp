#include "stdafx.h"
#include "Sector.h"

#include "IocpBase.h"
#include "Protocol.h"
#include "Session.h"
#include "SectorManager.h"
#include "NPC.h"
#include "Random.h"

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
			npc->SetName("������");
			npc->SetLevel(mSectorNum / gSectorsPerLine + 1);
			npc->SetMaxHP(mSectorNum / gSectorsPerLine + 3);
			npc->SetCurrentHP(npc->GetMaxHP());
			npc->SetRace(Race::Enemy1);
			npc->SetHostile(true);
			npc->SetBehaviorType(NpcBehaviorType::PEACE);
			npc->SetMoveType(NpcMoveType::FIXED);
			auto [x, y] = getAvailablePos(0);
			npc->SetPos(x, y);
			mActorIds.insert(freeID);
		}

		for (int i = quater; i < quater * 2; ++i)
		{
			auto npc = new NPC;
			auto freeID = GetFreeNpcID();
			gClients[freeID] = npc;
			npc->SetID(freeID);
			npc->SetName("����");
			npc->SetLevel(mSectorNum / gSectorsPerLine + 2);
			npc->SetMaxHP(mSectorNum / gSectorsPerLine + 5);
			npc->SetCurrentHP(npc->GetMaxHP());
			npc->SetRace(Race::Enemy2);
			npc->SetHostile(true);
			npc->SetBehaviorType(NpcBehaviorType::PEACE);
			npc->SetMoveType(NpcMoveType::FIXED);
			auto [x, y] = getAvailablePos(1);
			npc->SetPos(x, y);
			mActorIds.insert(freeID);
		}

		for (int i = quater * 2; i < quater * 3; ++i)
		{
			auto npc = new NPC;
			auto freeID = GetFreeNpcID();
			gClients[freeID] = npc;
			npc->SetID(freeID);
			npc->SetName("������");
			npc->SetLevel(mSectorNum / gSectorsPerLine + 3);
			npc->SetMaxHP(mSectorNum / gSectorsPerLine + 7);
			npc->SetCurrentHP(npc->GetMaxHP());
			npc->SetRace(Race::Enemy3);
			npc->SetHostile(true);
			npc->SetBehaviorType(NpcBehaviorType::PEACE);
			npc->SetMoveType(NpcMoveType::FIXED);
			auto [x, y] = getAvailablePos(2);
			npc->SetPos(x, y);
			mActorIds.insert(freeID);
		}

		for (int i = quater * 3; i < quater * 3 + 1; ++i)
		{
			auto npc = new NPC;
			auto freeID = GetFreeNpcID();
			gClients[freeID] = npc;
			npc->SetID(freeID);
			npc->SetName("��ī��");
			npc->SetLevel(mSectorNum / gSectorsPerLine + 4);
			npc->SetMaxHP(mSectorNum / gSectorsPerLine + 10);
			npc->SetCurrentHP(npc->GetMaxHP());
			npc->SetRace(Race::Enemy4);
			npc->SetHostile(true);
			npc->SetBehaviorType(NpcBehaviorType::PEACE);
			npc->SetMoveType(NpcMoveType::FIXED);
			auto [x, y] = getAvailablePos(3);
			npc->SetPos(x, y);
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

		auto targetPos = target->GetPos();
		const auto& targetName = target->GetName();

		std::unordered_set<id_type> actorIds;
		{
			WriteLockGuard guard = { mLock };
			mActorIds.insert(targetID);
			actorIds = mActorIds;
		}

		for (auto actorID : actorIds)
		{
			if (actorID == targetID) continue;

			auto actor = gClients[actorID];

			auto actorPos = actor->GetPos();
			bool bInView = isInView(targetPos, actorPos);

			if (bInView)
			{
				{
					WriteLockGuard guard = { target->ViewLock };
					target->ViewList.insert(actorID);
				}
				static_cast<Session*>(target)->SendAddObjectPacket(actorID);

				{
					WriteLockGuard guard = { actor->ViewLock };
					actor->ViewList.insert(targetID);
				}

				if (actorID < MAX_USER)
				{
					static_cast<Session*>(actor)->SendAddObjectPacket(targetID);
				}
			}
		}
	}

	void Sector::RemoveActor(Actor* target)
	{
		auto targetID = target->GetID();
		{
			WriteLockGuard guard = { mLock };
			if (0 == mActorIds.count(targetID))
			{
				MK_SLOG("RemoveActor() : Actor[{0}] is not in sector[{1}]", targetID,
					mSectorNum);
				return;
			}
			else
			{
				mActorIds.erase(targetID);
			}
		}

		std::unordered_set<id_type> viewList;
		{
			ReadLockGuard guard = { target->ViewLock };
			viewList = target->ViewList;
		}

		for (auto actorID : viewList)
		{
			static_cast<Session*>(target)->SendRemoveObjectPacket(actorID);

			auto actor = gClients[actorID];

			{
				ReadLockGuard guard = { actor->ViewLock };
				if (0 == (actor->ViewList).count(targetID))
				{
					continue;
				}
			}

			{
				WriteLockGuard guard = { actor->ViewLock };
				actor->ViewList.erase(targetID);
			}

			if (actorID < MAX_USER)
			{
				static_cast<Session*>(actor)->SendRemoveObjectPacket(targetID);
			}
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

		// ���� ViewList ����
		std::unordered_set<id_type> oldList;
		{
			ReadLockGuard guard = { target->ViewLock };
			oldList = target->ViewList;
		}

		// NearList ����
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
			(target->ViewLock).WriteLock();
			if (0 == target->ViewList.count(actorID))
			{
				(target->ViewList).insert(actorID);
				(target->ViewLock).WriteUnlock();
				static_cast<Session*>(target)->SendAddObjectPacket(actorID);
			}
			else
			{
				(target->ViewLock).WriteUnlock();
			}

			if (actorID < MAX_USER)
			{
				(gClients[actorID]->ViewLock).WriteLock();
				if (0 == (gClients[actorID]->ViewList).count(targetID))
				{
					(gClients[actorID]->ViewList).insert(targetID);
					(gClients[actorID]->ViewLock).WriteUnlock();
					static_cast<Session*>(gClients[actorID])->SendAddObjectPacket(targetID);
				}
				else
				{
					(gClients[actorID]->ViewLock).WriteUnlock();
					static_cast<Session*>(gClients[actorID])->SendMovePacket(targetID, 0);
				}
			}
		}

		for (auto actorID : oldList)
		{
			if (0 != nearList.count(actorID))
			{
				continue;
			}

			(target->ViewLock).WriteLock();
			if (0 != (target->ViewList).count(actorID))
			{
				(target->ViewList).erase(actorID);
				(target->ViewLock).WriteUnlock();
				static_cast<Session*>(target)->SendRemoveObjectPacket(actorID);
			}
			else
			{
				(target->ViewLock).WriteUnlock();
			}

			if (actorID < MAX_USER)
			{
				(gClients[actorID]->ViewLock).WriteLock();
				if (0 != (gClients[actorID]->ViewList).count(targetID))
				{
					gClients[actorID]->ViewList.erase(targetID);
					(gClients[actorID]->ViewLock).WriteUnlock();
					static_cast<Session*>(gClients[actorID])->SendRemoveObjectPacket(targetID);
				}
				else
				{
					(gClients[actorID]->ViewLock).WriteUnlock();
				}
			}
		}
	}

	void Sector::SendChat(Actor* target, const char chatType, std::string_view chat)
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
			if (actorID > MAX_USER) continue; // NPC

			static_cast<Session*>(gClients[actorID])->SendChatPacket(target->GetID(),
				chatType, chat);
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

	Sector::pos_type Sector::getAvailablePos(const int area)
	{
		const POINT leftTop = { (mSectorNum % gSectorsPerLine) * TILE_PER_SECTOR,
			(mSectorNum / gSectorsPerLine) * TILE_PER_SECTOR };

		short x = 0;
		short y = 0;

		switch (area)
		{
		case 0: // 2��и�
		{
			do
			{
				x = Random::RandInt(leftTop.x, leftTop.x + 19);
				y = Random::RandInt(leftTop.y, leftTop.y + 19);
			} while (isSolid(y, x));

			return { x, y };
		}

		case 1: // 1��и�
		{
			do
			{
				x = Random::RandInt(leftTop.x + 20, leftTop.x + 39);
				y = Random::RandInt(leftTop.y, leftTop.y + 19);
			} while (isSolid(y, x));

			return { x, y };
		}

		case 2: // 3��и�
		{
			do
			{
				x = Random::RandInt(leftTop.x, leftTop.x + 19);
				y = Random::RandInt(leftTop.y + 20, leftTop.y + 39);
			} while (isSolid(y, x));

			return { x, y };
		}

		case 3: // 4��и�
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
