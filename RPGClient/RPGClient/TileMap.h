#pragma once

struct Tile
{
	const TextureRegion& TileTex;
};

class TileMap
{
public:
	static void Init();
	static void LoadMapAsync(const String& mapFile);
	static void WaitMapLoading();
	static void LoadMap(const String& mapFile);

	static void RenderMap(const Point& playerPos);

private:
	static std::vector<std::vector<Tile>> sTileMap;
	static int32 sMapWidth;
	static int32 sMapHeight;
	static std::future<void> sTask;
};

