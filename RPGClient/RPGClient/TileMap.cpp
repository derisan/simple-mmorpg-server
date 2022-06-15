#include "stdafx.h"
#include "TileMap.h"

#include <fstream>

std::vector<std::vector<Tile>> TileMap::mTileMap;
s3d::int32 TileMap::mMapWidth;
s3d::int32 TileMap::mMapHeight;

void TileMap::LoadMap(const String& mapFile)
{
	std::ifstream file(mapFile.narrow());

	if (not file.is_open())
	{
		MK_ASSERT(false);
	}

	int32 width = 0;
	int32 height = 0;
	file >> width >> height;

	mMapWidth = width;
	mMapHeight = height;

	mTileMap.resize(height);

	const Texture tileset{ ASSET_PATH(TileSet.png) };

	int32 tileOffset = 0;
	int32 solidity = 0;
	for (int32 row = 0; row < height; ++row)
	{
		for (int32 col = 0; col < width; ++col)
		{
			file >> tileOffset >> solidity;

			mTileMap[row].emplace_back(tileset(tileOffset % 10 * 16, tileOffset / 10 * 16, 16, 16).scaled(2.0),
				solidity == 1 ? true : false);
		}
	}
}

void TileMap::RenderMap(const Point& position)
{
	for (int32 row = 0; row < 20; ++row)
	{
		for (int32 col = 0; col < 20; ++col)
		{
			mTileMap[row][col].TileTex.draw(col * 32, row * 32);
		}
	}
}

