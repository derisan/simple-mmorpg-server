#pragma once

#include <concurrent_unordered_set.h>

#include "TileMap.h"

namespace mk
{
	class Sector
	{
	public:
		using id_type = int;

		Sector(const std::vector<std::vector<Tile>>& tileMap, int sectorNum);

		void AddActor(const id_type id);

		void RemoveActor(const id_type id);

		void MoveActor(const id_type id, const char direction,
			const unsigned int clientTime);
		
	private:
		bool isSolid(const short row, const short col);

		bool isOutOfBound(const short x, const short y);

		void sendMovePacket(const id_type id, const short x, const short y,
			const unsigned int clientTime);

	private:
		concurrency::concurrent_unordered_set<id_type> mActors;
		const std::vector<std::vector<Tile>>& mTileMap;
		int mSectorNum = INVALID_VALUE;
	};
}
