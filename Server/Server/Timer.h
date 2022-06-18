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
		EV_REGEN_ENEMY,
	};

	struct TimerEvent
	{
		TimerEventType EventType = TimerEventType::EV_NONE;
		id_t ID = INVALID_VALUE;
		std::chrono::system_clock::time_point ActTime = {};
		char ExtraData[20] = {};

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
		static void AddEvent(const TimerEventType eventType, const id_t id, 
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

