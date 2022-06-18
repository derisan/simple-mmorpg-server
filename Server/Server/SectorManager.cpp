#include "stdafx.h"
#include "SectorManager.h"

#include "Actor.h"
#include "TileMap.h"
#include "Protocol.h"
#include "Sector.h"

namespace mk
{
	constexpr int MAX_SESSION_PER_SECTOR = 50;

	std::vector<std::vector<std::unique_ptr<Sector>>> SectorManager::sSectors;
	int gSectorsPerLine = 0;

	void SectorManager::Init()
	{
		TileMap::LoadTileMap("../../Assets/WorldMap.txt");
		const auto& tileMap = TileMap::GetTileMap();

		gSectorsPerLine = W_WIDTH / TILE_PER_SECTOR;
		sSectors.resize(gSectorsPerLine);

		for (int row = 0; row < gSectorsPerLine; ++row)
		{
			for (int col = 0; col < gSectorsPerLine; ++col)
			{
				sSectors[row].push_back(std::make_unique<Sector>(tileMap, (row * gSectorsPerLine) + col));
			}
		}
	}

	void SectorManager::AddActor(Actor* actor)
	{
		auto [x, y] = actor->GetPos();
		auto& sector = getSector(x, y);
		sector->AddActor(actor);
	}

	void SectorManager::RemoveActor(Actor* actor)
	{
		auto [x, y] = actor->GetPos();
		auto& sector = getSector(x, y);
		sector->RemoveActor(actor);
	}

	void SectorManager::MoveActor(Actor* actor, const char direction, const unsigned int clientTime /*= 0*/)
	{
		auto [x, y] = actor->GetPos();
		auto& sector = getSector(x, y);
		sector->MoveActor(actor, direction, clientTime);
	}

	void SectorManager::SendChat(Actor* actor, const char chatType, std::string_view chat)
	{
		auto [x, y] = actor->GetPos();
		auto& sector = getSector(x, y);
		sector->SendChatToViewList(actor, chatType, chat);
	}

	void SectorManager::DoAttack(Actor* actor)
	{
		auto [x, y] = actor->GetPos();
		auto& sector = getSector(x, y);
		sector->DoAttack(actor);
	}

	void SectorManager::SendStatChangeToViewList(Actor* actor)
	{
		auto [x, y] = actor->GetPos();
		auto& sector = getSector(x, y);
		sector->SendStatChangeToViewList(actor);
	}

	void SectorManager::ChangeSector(Actor* actor, const int prevSectorNum)
	{
		auto& prevSector = getSector(prevSectorNum);
		prevSector->RemoveActor(actor);
		
		auto [x, y] = actor->GetPos();
		auto& curSector = getSector(x, y);
		curSector->AddActor(actor);
	}

	void SectorManager::RegenEnemy(Actor* actor, const int sectorNum)
	{
		auto& sector = getSector(sectorNum);
		sector->RegenEnemy(actor);
	}

	std::pair<short, short> SectorManager::GetAvailablePos()
	{
		for (const auto& row : sSectors)
		{
			for (const auto& sector : row)
			{
				if (sector->GetNumSessions() < MAX_SESSION_PER_SECTOR)
				{
					auto sectorNum = sector->GetSectorNum();
					short left = (sectorNum % gSectorsPerLine) * TILE_PER_SECTOR;
					short top = (sectorNum / gSectorsPerLine) * TILE_PER_SECTOR;
					return { left + 4, top + 4 };
				}
			}
		}

		MK_ASSERT(false);
		return { -1, -1 };
	}

	std::unique_ptr<Sector>& SectorManager::getSector(const short x, const short y)
	{
		short row = (y / TILE_PER_SECTOR);
		short col = (x / TILE_PER_SECTOR);

		return sSectors[row][col];
	}

	std::unique_ptr<mk::Sector>& SectorManager::getSector(const int sectorNum)
	{
		short row = sectorNum / gSectorsPerLine;
		short col = sectorNum % gSectorsPerLine;

		return sSectors[row][col];
	}

}
