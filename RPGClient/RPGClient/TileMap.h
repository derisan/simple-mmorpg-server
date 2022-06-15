#pragma once

struct Tile
{
	TextureRegion TileTex = {};
};

class TileMap
{
public:
	static void LoadMapAsync(const String& mapFile);
	static void WaitMapLoading();
	static void LoadMap(const String& mapFile);

	static void RenderMap(const Point& playerPos);

private:
	static std::vector<std::vector<Tile>> mTileMap;
	static int32 mMapWidth;
	static int32 mMapHeight;
	static std::future<void> mTask;
};

