#pragma once

#include <queue>
#include <chrono>
#include <mutex>

#include "OverlappedEx.h"

namespace mk
{
	enum class TimerEventType
	{
		EV_NONE,
		EV_BIND_ACCEPT
	};

	struct TimerEvent
	{
		TimerEventType EventType = TimerEventType::EV_NONE;
		int32_t ID = INVALID_VALUE;
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
		static void AddEvent(const TimerEventType eventType, const int32_t id, 
			const std::chrono::system_clock::time_point actTime);
		static void Run();
		static void PushOverEx(OVERLAPPEDEX* overEx);

	private:
		static std::priority_queue<TimerEvent> mTimerQueue;
		static std::mutex mLock;
		static HANDLE mIocp;
		static std::unique_ptr<OverlappedPool> mPool;
	};
}

