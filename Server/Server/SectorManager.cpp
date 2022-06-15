#include "stdafx.h"
#include "SectorManager.h"

#include "Actor.h"
#include "TileMap.h"
#include "Protocol.h"

namespace mk
{
	constexpr int SECTOR_UNIT = 40;

	std::vector<std::vector<std::unique_ptr<Sector>>> SectorManager::sSectors;

	void SectorManager::Init()
	{
		TileMap::LoadTileMap("../../Assets/WorldMap.txt");
		const auto& tileMap = TileMap::GetTileMap();

		auto numlines = W_WIDTH / SECTOR_UNIT;
		sSectors.resize(numlines);

		for (int row = 0; row < numlines; ++row)
		{
			for (int col = 0; col < numlines; ++col)
			{
				sSectors[row].push_back(std::make_unique<Sector>(tileMap, row + col));
			}
		}
	}

	void SectorManager::AddActor(Actor* actor)
	{
		// TODO :
		// 해당 섹터는 기존의 액터들에게 ADD_OBJECT
		auto [x, y] = actor->GetPos();
		auto& sector = getSector(x, y);
		auto id = actor->GetID();
		sector->AddActor(id);
	}

	void SectorManager::MoveActor(Actor* actor, const char direction, const unsigned int clientTime /*= 0*/)
	{
		auto [x, y] = actor->GetPos();
		auto& sector = getSector(x, y);
		auto id = actor->GetID();
		sector->MoveActor(id, direction, clientTime);
	}

	std::unique_ptr<Sector>& SectorManager::getSector(const short x, const short y)
	{
		short row = (y / 40);
		short col = (x / 40);

		return sSectors[row][col];
	}
}
