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
		Sector(const std::vector<std::vector<Tile>>& tileMap, int sectorNum);

		void AddActor(Actor* target);

		void RemoveActor(Actor* target);

		void MoveActor(Actor* target, const char direction,
			const unsigned int clientTime);

		void SendChatToViewList(Actor* target, const char chatType, std::string_view chat);

		void DoAttack(Actor* hitter);

		void SendStatChangeToViewList(Actor* target);

		void RegenEnemy(Actor* actor);

		void RegenUser(Actor* actor);

		void SendNpcMoveToViewList(Actor* target);

		int GetNumSessions() const { return mNumSessions.load(); }

		int GetSectorNum() const { return mSectorNum; }
		
	private:
		bool isSolid(const short row, const short col);

		bool isOutOfBound(const short x, const short y);

		bool isInView(const vec2& aPos, const vec2& bPos);

		bool isInAttackRange(const vec2& hitterPos, const vec2& victimPos);

		vec2 getAvailablePos(const int race);

		void createEnemy(const int race, const int level);

	private:
		std::unordered_set<id_t> mActorIds;
		const std::vector<std::vector<Tile>>& mTileMap;
		int mSectorNum = INVALID_VALUE;
		std::atomic<int> mNumSessions = 0;

		SpinLock mLock = {};
	};
}
