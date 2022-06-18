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
		mPosX = INVALID_VALUE;
		mPosY = INVALID_VALUE;
		mLevel = INVALID_VALUE;
		mMaxHP = INVALID_VALUE;
		mCurrentHP = INVALID_VALUE;
		mRace = Race::None;
		mbAttack = true;
		mAttackPower = INVALID_VALUE;
		mExp = INVALID_VALUE;
	}
}