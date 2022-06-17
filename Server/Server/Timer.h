#pragma once

#include <queue>
#include <chrono>

#include "OverlappedEx.h"
#include "Lock.h"

namespace mk
{
	enum class TimerEventType
	{
		EV_NONE,
		EV_BIND_ACCEPT,
		EV_RESET_ATTACK,
	};

	struct TimerEvent
	{
		TimerEventType EventType = TimerEventType::EV_NONE;
		int ID = INVALID_VALUE;
		std::chrono::system_clock::time_point ActTime = {};

		bool operator < (const TimerEvent& other) const
		{
			return ActTime > other.ActTime;
		}
	};

	class Timer
	{
	public:
		static void Init(HANDLE iocp);
		static void AddEvent(const TimerEvent& ev);
		static void AddEvent(const TimerEventType eventType, const int id, 
			const std::chrono::system_clock::time_point actTime);
		static void Run();
		static void PushOverEx(OVERLAPPEDEX* overEx);

	private:
		static std::priority_queue<TimerEvent> sTimerQueue;
		static SpinLock sLock;
		static HANDLE sIocp;
		static std::unique_ptr<OverlappedPool> sPool;
	};
}

