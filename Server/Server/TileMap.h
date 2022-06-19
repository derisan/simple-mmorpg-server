#pragma once

namespace mk
{
	struct Tile
	{
		int TileType = INVALID_VALUE;
		int Solidity = INVALID_VALUE;
	};

	class TileMap
	{
	public:
		static void LoadTileMap(std::string_view mapFile);

		static const std::vector<std::vector<Tile>>& GetTileMap() { return sTileMap; }

		static bool IsSolid(const int row, const int col);

	private:
		static std::vector<std::vector<Tile>> sTileMap;
	};
}