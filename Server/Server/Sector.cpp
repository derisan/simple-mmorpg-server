#include "stdafx.h"
#include "Sector.h"

#include "IocpBase.h"
#include "Protocol.h"
#include "Session.h"
#include "SectorManager.h"

namespace mk
{
	enum
	{
		UP,
		DOWN,
		LEFT,
		RIGHT
	};

	Sector::Sector(const std::vector<std::vector<Tile>>& tileMap, int sectorNum)
		: mTileMap{ tileMap }
		, mSectorNum{ sectorNum }
	{

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
			if (actorID > MAX_USER) continue; // NPC

			Session* oldbie = static_cast<Session*>(gClients[actorID]);
			auto oldbiePos = oldbie->GetPos();

			bool bInView = isInView(targetPos, oldbiePos);

			if (bInView)
			{
				{
					WriteLockGuard guard = { oldbie->ViewLock };
					oldbie->ViewList.insert(targetID);
				}
				oldbie->SendAddObjectPacket(targetID, targetPos.first,
					targetPos.second, targetName);

				auto oldbieID = oldbie->GetID();
				{
					WriteLockGuard guard = { target->ViewLock };
					target->ViewList.insert(oldbieID);
				}
				const auto& oldbieName = oldbie->GetName();
				static_cast<Session*>(target)->SendAddObjectPacket(oldbieID,
					oldbiePos.first, oldbiePos.second, oldbieName);
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

			{
				ReadLockGuard guard = { gClients[actorID]->ViewLock };
				if (0 == (gClients[actorID]->ViewList).count(targetID))
				{
					continue;
				}
			}

			if (actorID < MAX_USER)
			{
				{
					WriteLockGuard guard = { gClients[actorID]->ViewLock };
					gClients[actorID]->ViewList.erase(targetID);
				}
				static_cast<Session*>(gClients[actorID])->SendRemoveObjectPacket(targetID);
			}
			else
			{
				// TODO : NPC things
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
		static_cast<Session*>(target)->SendMovePacket(targetID, x, y, clientTime);

		bool bOut = isOutOfBound(x, y);
		if (bOut)
		{
			SectorManager::ChangeSector(target, mSectorNum);
			return;
		}

		// 기존 ViewList 복사
		std::unordered_set<id_type> oldList;
		{
			ReadLockGuard guard = { target->ViewLock };
			oldList = target->ViewList;
		}

		// NearList 생성
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
				auto [x, y] = gClients[actorID]->GetPos();
				const auto& name = gClients[actorID]->GetName();
				static_cast<Session*>(target)->SendAddObjectPacket(actorID,
					x, y, name);
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
					static_cast<Session*>(gClients[actorID])->SendAddObjectPacket(targetID,
						x, y, target->GetName());
				}
				else
				{
					(gClients[actorID]->ViewLock).WriteUnlock();
					static_cast<Session*>(gClients[actorID])->SendMovePacket(targetID,
						x, y, 0);
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
}
