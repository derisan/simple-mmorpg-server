#include "stdafx.h"
#include "TileMap.h"

#include <fstream>

std::vector<std::vector<Tile>> TileMap::sTileMap;
s3d::int32 TileMap::sMapWidth;
s3d::int32 TileMap::sMapHeight;
std::future<void> TileMap::sTask;

std::unordered_map<int32, TextureRegion> gTileRegions;

void TileMap::Init()
{
	const Texture tileset{ ASSET_PATH(TileSet.png) };

	for (int32 i = 0; i < 30; ++i)
	{
		gTileRegions[i] = tileset(i % 10 * 16, static_cast<int32>(i / 10) * 16,
			16, 16).scaled(2.0);
	}
}

void TileMap::LoadMapAsync(const String& mapFile)
{
	sTask = std::async(std::launch::async, TileMap::LoadMap, mapFile);
}

void TileMap::WaitMapLoading()
{
	if (not sTask.valid())
	{
		return;
	}

	sTask.wait();
}

void TileMap::LoadMap(const String& mapFile)
{
	sMapWidth = 2000;
	sMapHeight = 2000;

	std::ifstream file(mapFile.narrow());

	if (not file.is_open())
	{
		MK_ASSERT(false);
	}

	int32 width = 0;
	int32 height = 0;
	file >> width >> height; // (width, height) = (40, 40)

	std::vector<std::vector<Tile>> tilemap;
	tilemap.resize(height);
	
	for (int32 row = 0; row < height; ++row)
	{
		for (int32 col = 0; col < width; ++col)
		{
			int32 tileNumber = 0;
			int32 solidity = 0;

			file >> tileNumber >> solidity;

			tilemap[row].emplace_back(gTileRegions[tileNumber]);
		}
	}

	sTileMap.resize(sMapWidth);
	for (int32 row = 0; row < sMapHeight; ++row)
	{
		for (int32 col = 0; col < sMapWidth; ++col)
		{
			sTileMap[row].push_back(tilemap[row % 40][col % 40]);
		}
	}
}

void TileMap::RenderMap(const Point& playerPos)
{
	int32 left = 0;
	if (playerPos.x > 19)
	{
		left = (playerPos.x / 20) * 20;
	}

	int32 top = 0;
	if (playerPos.y > 19)
	{
		top = (playerPos.y / 20) * 20;
	}


	for (int32 row = top; row < top + 20; ++row)
	{
		for (int32 col = left; col < left + 20; ++col)
		{
			sTileMap[row][col].TileTex.draw((col % 20) * 32, (row % 20) * 32);
		}
	}
}

