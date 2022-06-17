#pragma once

#include <unordered_set>

#include "TileMap.h"
#include "Lock.h"

namespace mk
{
	class Actor;

	class Sector
	{
	public:
		using id_type = int;
		using pos_type = std::pair<short, short>;

		Sector(const std::vector<std::vector<Tile>>& tileMap, int sectorNum);

		void AddActor(Actor* target);

		void RemoveActor(Actor* target);

		void MoveActor(Actor* target, const char direction,
			const unsigned int clientTime);

		void SendChat(Actor* target, const char chatType, std::string_view chat);

		void DoAttack(Actor* hitter);

		void SendStatChange(Actor* target);
		
	private:
		bool isSolid(const short row, const short col);

		bool isOutOfBound(const short x, const short y);

		bool isInView(const pos_type& aPos, const pos_type& bPos);

		bool isInAttackRange(const pos_type& hitterPos, const pos_type& victimPos);

		pos_type getAvailablePos(const int area);

	private:
		std::unordered_set<id_type> mActorIds;
		const std::vector<std::vector<Tile>>& mTileMap;
		int mSectorNum = INVALID_VALUE;

		SpinLock mLock = {};
	};
}
