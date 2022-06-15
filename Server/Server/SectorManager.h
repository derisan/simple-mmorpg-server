#pragma once

namespace mk
{
	class Actor;
	class Sector;

	class SectorManager
	{
	public:
		static void Init();

		static void AddActor(Actor* actor);
		
		static void MoveActor(Actor* actor, const char direction, 
			const unsigned int clientTime = 0);
		
	private:
		static std::unique_ptr<Sector>& getSector(const short x, const short y);

	private:
		static std::vector<std::vector<std::unique_ptr<Sector>>> sSectors;
	};
}
