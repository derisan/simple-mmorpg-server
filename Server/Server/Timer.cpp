#include "stdafx.h"
#include "Timer.h"

namespace mk
{
	using namespace std::chrono;

	constexpr milliseconds TIMER_SLEEP_TIME = 33ms;

	std::priority_queue<mk::TimerEvent> Timer::sTimerQueue;
	SpinLock Timer::sLock;
	HANDLE Timer::sIocp = INVALID_HANDLE_VALUE;
	std::unique_ptr<mk::OverlappedPool> Timer::sPool = nullptr;

	void Timer::Init(HANDLE iocp)
	{
		sIocp = iocp;
		sPool = std::make_unique<mk::OverlappedPool>();
	}

	void Timer::AddEvent(const TimerEvent& ev)
	{
		WriteLockGuard guard = { sLock };
		sTimerQueue.push(ev);
	}

	void Timer::AddEvent(const TimerEventType eventType, const int id, 
		const std::chrono::system_clock::time_point actTime)
	{
		WriteLockGuard guard = { sLock };
		sTimerQueue.emplace(eventType, id, actTime);
	}

	void Timer::Run()
	{
		while (true)
		{
			sLock.WriteLock();
			if (sTimerQueue.empty())
			{
				sLock.WriteUnlock();
				std::this_thread::sleep_for(TIMER_SLEEP_TIME);
				continue;
			}

			const TimerEvent ev = sTimerQueue.top();
			if (ev.ActTime > system_clock::now())
			{
				sLock.WriteUnlock();
				std::this_thread::sleep_for(TIMER_SLEEP_TIME);
				continue;
			}
			sTimerQueue.pop();
			sLock.WriteUnlock();

			switch (ev.EventType)
			{
			case TimerEventType::EV_BIND_ACCEPT:
			{
				OVERLAPPEDEX* overEx = sPool->Pop();
				ZeroMemory(overEx, sizeof(OVERLAPPEDEX));
				overEx->OpType = OperationType::TIMER_BIND_ACCEPT;
				PostQueuedCompletionStatus(sIocp, 1, ev.ID, &overEx->Overlapped);
				break;
			}

			case TimerEventType::EV_RESET_ATTACK:
			{
				OVERLAPPEDEX* overEx = sPool->Pop();
				ZeroMemory(overEx, sizeof(OVERLAPPEDEX));
				overEx->OpType = OperationType::TIMER_RESET_ATTACK;
				PostQueuedCompletionStatus(sIocp, 1, ev.ID, &overEx->Overlapped);
				break;
			}

			default:
				MK_ASSERT(false);
				break;
			}
		}
	}

	void Timer::PushOverEx(OVERLAPPEDEX* overEx)
	{
		sPool->Push(overEx);
	}
}