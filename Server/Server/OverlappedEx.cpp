#include "stdafx.h"
#include "OverlappedEx.h"

namespace mk
{
	constexpr int MAX_OVEREX_NUM = 100;

	OverlappedPool::OverlappedPool()
	{
		for (int i = 0; i < MAX_OVEREX_NUM; ++i)
		{
			auto overEX = new OVERLAPPEDEX{};
			mPool.push(overEX);
		}
	}

	OverlappedPool::~OverlappedPool()
	{
		OVERLAPPEDEX* overEX = nullptr;
		while (mPool.try_pop(overEX))
		{
			delete overEX;
			overEX = nullptr;
		}
		mPool.clear();
	}

	mk::OVERLAPPEDEX* OverlappedPool::Pop()
	{
		OVERLAPPEDEX* overEX = nullptr;
		if (NOT mPool.try_pop(overEX))
		{
			overEX = new OVERLAPPEDEX{};
		}
		return overEX;
	}

	void OverlappedPool::Push(OVERLAPPEDEX* overEX)
	{
		mPool.push(overEX);
	}
}
