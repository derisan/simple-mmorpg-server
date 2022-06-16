#pragma once

namespace mk
{
	constexpr int TILE_PER_SECTOR = 40;
	extern int gSectorsPerLine;

	class Actor;
	class Sector;

	class SectorManager
	{
	public:
		static void Init();

		static void AddActor(Actor* actor);

		static void RemoveActor(Actor* actor);
		
		static void MoveActor(Actor* actor, const char direction, 
			const unsigned int clientTime = 0);
		
	private:
		static std::unique_ptr<Sector>& getSector(const short x, const short y);

	private:
		static std::vector<std::vector<std::unique_ptr<Sector>>> sSectors;
	};
}
