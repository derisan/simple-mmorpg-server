#pragma once

#include <unordered_set>

#include "Lock.h"
#include "Protocol.h"

namespace mk
{
	class Actor
	{
	public:
		void Disconnect();

	public:
		int GetID() const { return mID; }
		void SetID(const int value) { mID = value; }
		std::string GetName() const { return mName; }
		const std::string& GetName() { return mName; }
		void SetName(std::string_view value) { mName = value; }
		short GetX() const { return mPosX; }
		short GetY() const { return mPosY; }
		std::pair<short, short> GetPos() const { return { mPosX, mPosY }; }
		void SetX(const short value) { mPosX = value; }
		void SetY(const short value) { mPosY = value; }
		void SetPos(const std::pair<short, short>& value) { mPosX = value.first; mPosY = value.second; }
		void SetPos(const short x, const short y) { mPosX = x; mPosY = y; }
		int GetLevel() const { return mLevel; }
		void SetLevel(const int value) { mLevel = value; }
		int GetMaxHP() const { return mMaxHP; }
		void SetMaxHP(const int value) { mMaxHP = value; }
		int GetCurrentHP() const { return mCurrentHP; }
		void SetCurrentHP(const int value) { mCurrentHP = std::clamp(value, 0, mMaxHP); }
		short GetRace() const { return mRace; }
		void SetRace(const short race) { mRace = race; }

	public:
		SpinLock ActorLock = {};
		SpinLock ViewLock = {};
		std::unordered_set<int> ViewList;

	private:
		int mID = INVALID_VALUE;
		std::string mName = {};
		short mPosX = INVALID_VALUE;
		short mPosY = INVALID_VALUE;
		short mLevel = INVALID_VALUE;
		int mMaxHP = INVALID_VALUE;
		int mCurrentHP = INVALID_VALUE;
		short mRace = Race::None;
	};
}