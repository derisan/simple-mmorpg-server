#pragma once

#include <unordered_set>

#include "Lock.h"
#include "Protocol.h"

namespace mk
{
	class Actor
	{
	public:
		virtual ~Actor() = default;

		virtual void Shutdown();
		virtual void Tick() {};

		virtual bool AddToViewList(const id_t id, const bool bSendMove = false) abstract;
		virtual bool RemoveFromViewList(const id_t id) abstract;

	public:
		id_t GetID() const { return mID; }
		void SetID(const int value) { mID = value; }
		std::string GetName() const { return mName; }
		const std::string& GetName() { return mName; }
		void SetName(std::string_view value) { mName = value; }
		short GetX() const { return mPos.x; }
		short GetY() const { return mPos.y; }
		vec2 GetPos() const { return mPos; }
		const vec2& GetPos() { return mPos; }
		void SetX(const short value) { mPos.x = value; }
		void SetY(const short value) { mPos.y = value; }
		void SetPos(const vec2& value) { mPos = value; }
		void SetPos(const short x, const short y) { mPos = vec2{ x, y }; }
		int GetLevel() const { return mLevel; }
		void SetLevel(const int value) { mLevel = value; }
		int GetMaxHP() const { return mMaxHP; }
		void SetMaxHP(const int value) { mMaxHP = value; }
		int GetCurrentHP() const { return mCurrentHP; }
		void SetCurrentHP(const int value) { mCurrentHP = std::clamp(value, 0, mMaxHP); }
		short GetRace() const { return mRace; }
		void SetRace(const short race) { mRace = race; }
		void SetAttackPower(const int value) { mAttackPower = value; }
		int GetAttackPower() const { return mAttackPower; }
		void SetExp(const int value) { mExp = value; }
		int GetExp() const { return mExp; }
		void SetActive(const bool value) { mbActive = value; }
		bool IsActive() const { return mbActive; }

	public:
		SpinLock ActorLock = {};
		SpinLock ViewLock = {};
		std::unordered_set<int> ViewList;

	private:
		id_t mID = INVALID_VALUE;
		std::string mName = {};
		vec2 mPos = {};
		short mLevel = INVALID_VALUE;
		int mMaxHP = INVALID_VALUE;
		int mCurrentHP = INVALID_VALUE;
		short mRace = Race::None;
		int mAttackPower = INVALID_VALUE;
		int mExp = INVALID_VALUE;
		bool mbActive = false;
	};
}