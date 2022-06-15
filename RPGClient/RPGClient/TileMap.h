#pragma once

struct Tile
{
	TextureRegion TileTex = {};
	bool IsSolid = false;
};

class TileMap
{
public:
	static void LoadMap(const String& mapFile);

	static void RenderMap(const Point& position);

private:
	static std::vector<std::vector<Tile>> mTileMap;
	static int32 mMapWidth;
	static int32 mMapHeight;
};

