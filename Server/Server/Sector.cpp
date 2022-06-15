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
		auto [targetX, targetY] = target->GetPos();
		const auto& targetName = target->GetName();

		// TODO :
		// 시야 내 액터들에게 ADD_OBJECT
		// USER 및 NPC Viewlist 갱신
		for (auto actor : mActors)
		{
			if (actor < MAX_USER)
			{
				Session* oldbie = static_cast<Session*>(gClients[actor]);

				oldbie->SendAddObjectPacket(targetID, targetX, targetY, targetName);

				auto [obX, obY] = oldbie->GetPos();
				const auto& obName = oldbie->GetName();

				static_cast<Session*>(target)->SendAddObjectPacket(actor,
					obX, obY, obName);
			}
		}

		mActors.insert(targetID);
	}

	void Sector::RemoveActor(Actor* target)
	{
		// TODO : target 유효성 검사
		auto targetID = target->GetID();
		mActors.unsafe_erase(targetID);

		// TODO :
		// 시야 내 액터들에게 REMOVE_OBJECT
		for (auto actor : mActors)
		{
			if (actor < MAX_USER)
			{
				static_cast<Session*>(gClients[actor])->SendRemoveObjectPacket(targetID);
			}
		}
	}

	void Sector::MoveActor(Actor* target, const char direction,
		const unsigned int clientTime)
	{
		auto targetID = target->GetID();

		if (auto iter = mActors.find(targetID); iter != mActors.end())
		{
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

			bool bOut = isOutOfBound(x, y);
			if (bOut)
			{
				static_cast<Session*>(target)->SendMovePacket(targetID, x, y, clientTime);
				RemoveActor(target);
				SectorManager::AddActor(target);
				return;
			}

			sendMovePacket(targetID, x, y, clientTime);
		}
		else
		{
			MK_INFO("Actor id[{0}]: is not in sector[{1}]", targetID, mSectorNum);
		}
	}

	bool Sector::isSolid(const short row, const short col)
	{
		return mTileMap[row % 40][col % 40].Solidity == 1;
	}

	bool Sector::isOutOfBound(const short x, const short y)
	{
		const POINT leftTop = { (mSectorNum % 50) * 40, (mSectorNum / 50) * 40 };

		if (x < leftTop.x)
		{
			return true;
		}
		
		if (x > leftTop.x + 39)
		{
			return true;
		}

		if (y < leftTop.y)
		{
			return true;
		}

		if (y > leftTop.y + 39)
		{
			return true;
		}

		return false;
	}

	void Sector::sendMovePacket(const id_type id, const short x, const short y,
		const unsigned int clientTime)
	{
		for (auto inSectorID : mActors)
		{
			if (inSectorID < MAX_USER)
			{
				static_cast<Session*>(gClients[inSectorID])->SendMovePacket(id, x, y,
					clientTime);
			}
		}
	}
}
