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

	void Sector::AddActor(const id_type id)
	{
		mActors.insert(id);

		// TODO :
		// 시야 내 액터들에게 ADD_OBJECT
	}

	void Sector::RemoveActor(const id_type id)
	{
		mActors.unsafe_erase(id);

		// TODO :
		// 시야 내 액터들에게 REMOVE_OBJECT
	}

	void Sector::MoveActor(const id_type id, const char direction,
		const unsigned int clientTime)
	{
		if (auto iter = mActors.find(id); iter != mActors.end())
		{
			auto actor = gClients[id];
			auto [x, y] = actor->GetPos();

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

			actor->SetPos(x, y);

			bool bOut = isOutOfBound(x, y);
			if (bOut)
			{
				static_cast<Session*>(actor)->SendMovePacket(id, x, y, clientTime);
				RemoveActor(id);
				SectorManager::AddActor(actor);
				return;
			}

			sendMovePacket(id, x, y, clientTime);
		}
		else
		{
			MK_INFO("Actor id[{0}]: is not in sector[{1}]", id, mSectorNum);
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
