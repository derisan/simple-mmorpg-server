#include "stdafx.h"
#include "Actor.h"

namespace mk
{
	void Actor::Shutdown()
	{
		WriteLockGuard guard = { ViewLock };
		mbActive = false;
		ViewList.clear();
		mName = {};
		mPos = {};
		mLevel = INVALID_VALUE;
		mMaxHP = INVALID_VALUE;
		mCurrentHP = INVALID_VALUE;
		mRace = Race::None;
		mbAttack = true;
		mAttackPower = INVALID_VALUE;
		mExp = INVALID_VALUE;
	}
}