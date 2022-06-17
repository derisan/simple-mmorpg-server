#pragma once

namespace mk
{
	class SpinLock
	{
	public:
		SpinLock();

		void ReadLock();

		void ReadUnlock();

		void WriteLock();

		void WriteUnlock();

	private:
		SRWLOCK mLock;
	};

	class ReadLockGuard
	{
	public:
		ReadLockGuard(SpinLock& lock)
			: mLock(lock)
		{
			mLock.ReadLock();
		}

		~ReadLockGuard()
		{
			mLock.ReadUnlock();
		}

	private:
		SpinLock& mLock;
	};

	class WriteLockGuard
	{
	public:
		WriteLockGuard(SpinLock& lock)
			: mLock(lock)
		{
			mLock.WriteLock();
		}

		~WriteLockGuard()
		{
			mLock.WriteUnlock();
		}

	private:
		SpinLock& mLock;
	};
}

