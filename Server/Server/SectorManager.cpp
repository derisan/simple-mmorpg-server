#include "stdafx.h"
#include "SectorManager.h"

#include "Actor.h"
#include "TileMap.h"
#include "Protocol.h"
#include "Sector.h"

namespace mk
{
	constexpr int TILE_PER_SECTOR = 40;

	std::vector<std::vector<std::unique_ptr<Sector>>> SectorManager::sSectors;

	void SectorManager::Init()
	{
		TileMap::LoadTileMap("../../Assets/WorldMap.txt");
		const auto& tileMap = TileMap::GetTileMap();

		auto sectorsPerLine = W_WIDTH / TILE_PER_SECTOR;
		sSectors.resize(sectorsPerLine);

		for (int row = 0; row < sectorsPerLine; ++row)
		{
			for (int col = 0; col < sectorsPerLine; ++col)
			{
				sSectors[row].push_back(std::make_unique<Sector>(tileMap, (row * 50) + col));
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

	std::unique_ptr<Sector>& SectorManager::getSector(const short x, const short y)
	{
		short row = (y / 40);
		short col = (x / 40);

		return sSectors[row][col];
	}
}
