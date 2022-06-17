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
		
	private:
		bool isSolid(const short row, const short col);

		bool isOutOfBound(const short x, const short y);

		bool isInView(const pos_type& aPos, const pos_type& bPos);

	private:
		std::unordered_set<id_type> mActors;
		const std::vector<std::vector<Tile>>& mTileMap;
		int mSectorNum = INVALID_VALUE;

		SpinLock mLock = {};
	};
}
