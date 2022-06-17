#include "stdafx.h"
#include "Lock.h"

namespace mk
{
	SpinLock::SpinLock()
	{
		InitializeSRWLock(&mLock);
	}

	void SpinLock::ReadLock()
	{
		AcquireSRWLockShared(&mLock);
	}

	void SpinLock::ReadUnlock()
	{
		ReleaseSRWLockShared(&mLock);
	}

	void SpinLock::WriteLock()
	{
		AcquireSRWLockExclusive(&mLock);
	}

	void SpinLock::WriteUnlock()
	{
		ReleaseSRWLockExclusive(&mLock);
	}

}