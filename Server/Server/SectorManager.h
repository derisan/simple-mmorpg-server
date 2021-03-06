#pragma once

#include <string_view>

namespace mk
{
	constexpr int TILE_PER_SECTOR = 40;
	extern int gSectorsPerLine;
	extern lua_State* gLuaVM;

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

		static void SendChat(Actor* actor, const char chatType, std::string_view chat);

		static void DoAttack(Actor* actor);

		static void SendStatChangeToViewList(Actor* actor);
		
		static void SendNpcMoveToViewList(Actor* actor);

		static void ChangeSector(Actor* actor, const int prevSectorNum);

		static void RegenEnemy(Actor* actor, const int sectorNum);

		static void RegenUser(Actor* actor);

		static std::pair<short, short> GetAvailablePos();
		
	private:
		static std::unique_ptr<Sector>& getSector(const short x, const short y);
		static std::unique_ptr<Sector>& getSector(const int sectorNum);

	private:
		static std::vector<std::vector<std::unique_ptr<Sector>>> sSectors;
	};
}
