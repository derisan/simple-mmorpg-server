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
		auto targetPos = target->GetPos();
		const auto& targetName = target->GetName();

		{
			WriteLockGuard guard = { mLock };
			mActors.insert(targetID);
		}

		{
			ReadLockGuard guard = { mLock };
			for (auto actorID : mActors)
			{
				if (actorID == targetID)
				{
					continue;
				}

				if (actorID < MAX_USER)
				{
					Session* oldbie = static_cast<Session*>(gClients[actorID]);
					auto oldbiePos = oldbie->GetPos();

					bool bInView = isInView(targetPos, oldbiePos);

					if (bInView)
					{
						{
							WriteLockGuard guard = { oldbie->ActorLock };
							oldbie->ViewList.insert(targetID);
						}
						oldbie->SendAddObjectPacket(targetID, targetPos.first,
							targetPos.second, targetName);

						auto oldbieID = oldbie->GetID();
						const auto& oldbieName = oldbie->GetName();
						{
							WriteLockGuard guard = { target->ActorLock };
							target->ViewList.insert(oldbieID);
						}
						static_cast<Session*>(target)->SendAddObjectPacket(oldbieID,
							oldbiePos.first, oldbiePos.second, oldbieName);
					}
				}
			}
		}
	}

	void Sector::RemoveActor(Actor* target)
	{
		auto targetID = target->GetID();
		mActors.erase(targetID);

		std::unordered_set<id_type> viewList;
		{
			ReadLockGuard guard = { target->ActorLock };
			viewList = target->ViewList;
		}

		for (auto actorID : viewList)
		{
			WriteLockGuard guard = { target->ActorLock };
			if (0 == (target->ViewList).count(actorID))
			{
				continue;
			}

			if (actorID < MAX_USER)
			{
				static_cast<Session*>(gClients[actorID])->SendRemoveObjectPacket(targetID);
				static_cast<Session*>(target)->SendRemoveObjectPacket(actorID);

				target->ViewList.erase(actorID);

				{
					WriteLockGuard guard = { gClients[actorID]->ActorLock };
					gClients[actorID]->ViewList.erase(targetID);
				}
			}
		}
	}

	void Sector::MoveActor(Actor* target, const char direction,
		const unsigned int clientTime)
	{
		auto targetID = target->GetID();

		{
			mLock.ReadLock();
			auto count = mActors.count(targetID);
			if (count == 0)
			{
				mLock.ReadUnLock();
				MK_SLOG("Actor id[{0}]: is not in sector[{1}]", targetID, mSectorNum);
				AddActor(target);
				return;
			}
			mLock.ReadUnLock();
		}

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
			RemoveActor(target);
			SectorManager::AddActor(target);
			return;
		}

		std::unordered_set<id_type> oldList;
		{
			ReadLockGuard guard = { target->ActorLock };
			oldList = target->ViewList;
		}

		std::unordered_set<id_type> nearList;
		for (auto actorID : mActors)
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

		for (auto actorID : nearList)
		{
			(target->ActorLock).WriteLock();
			if (0 == target->ViewList.count(actorID))
			{
				(target->ViewList).insert(actorID);
				(target->ActorLock).WriteUnlock();
				auto [x, y] = gClients[actorID]->GetPos();
				const auto& name = gClients[actorID]->GetName();
				static_cast<Session*>(target)->SendAddObjectPacket(actorID,
					x, y, name);
			}
			else
			{
				(target->ActorLock).WriteUnlock();
				//static_cast<Session*>(target)->SendMovePacket()
			}

			if (actorID < MAX_USER)
			{
				(gClients[actorID]->ActorLock).WriteLock();
				if (0 == (gClients[actorID]->ViewList).count(targetID))
				{
					(gClients[actorID]->ViewList).insert(targetID);
					(gClients[actorID]->ActorLock).WriteUnlock();
					static_cast<Session*>(gClients[actorID])->SendAddObjectPacket(targetID,
						x, y, target->GetName());
				}
				else
				{
					(gClients[actorID]->ActorLock).WriteUnlock();
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

			(target->ActorLock).WriteLock();
			if (0 != (target->ViewList).count(actorID))
			{
				(target->ViewList).erase(actorID);
				(target->ActorLock).WriteUnlock();
				static_cast<Session*>(target)->SendRemoveObjectPacket(actorID);
			}
			else
			{
				(target->ActorLock).WriteUnlock();
			}

			if (actorID < MAX_USER)
			{
				(gClients[actorID]->ActorLock).WriteLock();
				if (0 != (gClients[actorID]->ViewList).count(targetID))
				{
					gClients[actorID]->ViewList.erase(targetID);
					(gClients[actorID]->ActorLock).WriteUnlock();
					static_cast<Session*>(gClients[actorID])->SendRemoveObjectPacket(targetID);
				}
				else
				{
					(gClients[actorID]->ActorLock).WriteUnlock();
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
