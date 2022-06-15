#include "stdafx.h"
#include "TileMap.h"

#include <fstream>

std::vector<std::vector<Tile>> TileMap::mTileMap;
s3d::int32 TileMap::mMapWidth;
s3d::int32 TileMap::mMapHeight;
std::future<void> TileMap::mTask;

void TileMap::LoadMapAsync(const String& mapFile)
{
	mTask = std::async(std::launch::async, TileMap::LoadMap, mapFile);
}

void TileMap::WaitMapLoading()
{
	if (not mTask.valid())
	{
		return;
	}

	mTask.wait();
}

void TileMap::LoadMap(const String& mapFile)
{
	// TODO : width, height protocol.h에 있는 전역 변수로 교체
	mMapWidth = 2000;
	mMapHeight = 2000;

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
	const Texture tileset{ ASSET_PATH(TileSet.png) };
	
	for (int32 row = 0; row < height; ++row)
	{
		for (int32 col = 0; col < width; ++col)
		{
			int32 tileOffset = 0;
			int32 solidity = 0;

			file >> tileOffset >> solidity;

			tilemap[row].emplace_back(tileset(tileOffset % 10 * 16, tileOffset / 10 * 16, 16, 16).scaled(2.0));
		}
	}

	mTileMap.resize(mMapWidth);
	for (int32 row = 0; row < mMapHeight; ++row)
	{
		for (int32 col = 0; col < mMapWidth; ++col)
		{
			mTileMap[row].push_back(tilemap[row % 40][col % 40]);
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
			mTileMap[row][col].TileTex.draw((col % 20) * 32, (row % 20) * 32);
		}
	}
}

