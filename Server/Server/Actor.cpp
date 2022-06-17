#include "stdafx.h"
#include "Actor.h"

namespace mk
{
	void Actor::Disconnect()
	{
		{
			WriteLockGuard guard = { ViewLock };
			ViewList.clear();
		}
		mName = {};
		mPosX = 0;
		mPosY = 0;
	}
}