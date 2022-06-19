#include "stdafx.h"
#include "TileMap.h"

#include <fstream>

namespace mk
{
	std::vector<std::vector<mk::Tile>> TileMap::sTileMap;

	void TileMap::LoadTileMap(std::string_view mapFile)
	{
		std::ifstream file(mapFile.data());

		if (NOT file.is_open())
		{
			MK_ASSERT(false);
		}

		int width = 0;
		int height = 0;
		file >> width >> height; // (width, height) = (40, 40)

		sTileMap.resize(height);

		for (int row = 0; row < height; ++row)
		{
			for (int col = 0; col < width; ++col)
			{
				int tileType = 0;
				int solidity = 0;

				file >> tileType >> solidity;

				sTileMap[row].emplace_back(tileType, solidity);
			}
		}
	}

	bool TileMap::IsSolid(const int row, const int col)
	{
		if(row < 0 || row >= sTileMap.size())
		{
			return true;
		}

		if (col < 0 || col >= sTileMap.size())
		{
			return true;
		}

		return sTileMap[row][col].Solidity == 1;
	}

}