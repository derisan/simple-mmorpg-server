#include "stdafx.h"
#include "Timer.h"

namespace mk
{
	using namespace std::chrono;

	constexpr milliseconds TIMER_SLEEP_TIME = 33ms;

	std::priority_queue<mk::TimerEvent> Timer::mTimerQueue;
	std::mutex Timer::mLock;
	HANDLE Timer::mIocp = INVALID_HANDLE_VALUE;
	std::unique_ptr<mk::OverlappedPool> Timer::mPool = nullptr;

	void Timer::Init(HANDLE iocp)
	{
		mIocp = iocp;
		mPool = std::make_unique<mk::OverlappedPool>();
	}

	void Timer::AddEvent(const TimerEvent& ev)
	{
		std::lock_guard guard{ mLock };
		mTimerQueue.push(ev);
	}

	void Timer::AddEvent(const TimerEventType eventType, const int32_t id, 
		const std::chrono::system_clock::time_point actTime)
	{
		std::lock_guard guard{ mLock };
		mTimerQueue.emplace(eventType, id, actTime);
	}

	void Timer::Run()
	{
		while (true)
		{
			mLock.lock();
			if (mTimerQueue.empty())
			{
				mLock.unlock();
				std::this_thread::sleep_for(TIMER_SLEEP_TIME);
				continue;
			}

			const TimerEvent ev = mTimerQueue.top();
			if (ev.ActTime > system_clock::now())
			{
				mLock.unlock();
				std::this_thread::sleep_for(TIMER_SLEEP_TIME);
				continue;
			}
			mTimerQueue.pop();
			mLock.unlock();

			switch (ev.EventType)
			{
			case TimerEventType::EV_BIND_ACCEPT:
			{
				OVERLAPPEDEX* overEx = mPool->Pop();
				ZeroMemory(overEx, sizeof(OVERLAPPEDEX));
				overEx->OpType = OperationType::TIMER_BIND_ACCEPT;
				PostQueuedCompletionStatus(mIocp, 1, ev.ID, &overEx->Overlapped);
			}
				break;

			default:
				MK_ASSERT(false);
				break;
			}
		}
	}

	void Timer::PushOverEx(OVERLAPPEDEX* overEx)
	{
		mPool->Push(overEx);
	}
}