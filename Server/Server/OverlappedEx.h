#pragma once

#include <concurrent_queue.h>

namespace mk
{
	constexpr uint32_t SEND_BUFFER_SIZE = 200;
	
	enum class OperationType
	{
		OP_NONE,
		OP_ACCEPT,
		OP_RECV,
		OP_SEND,
		TIMER_BIND_ACCEPT,
	};

	struct OVERLAPPEDEX
	{
		WSAOVERLAPPED Overlapped = {};
		WSABUF WsaBuf = {};
		OperationType OpType = OperationType::OP_NONE;
		int32_t ID = INVALID_VALUE;
		char SendBuffer[SEND_BUFFER_SIZE] = {};
	};

	class OverlappedPool
	{
	public:
		OverlappedPool();
		~OverlappedPool();

		OVERLAPPEDEX* Pop();

		void Push(OVERLAPPEDEX* overEX);

	private:
		concurrency::concurrent_queue<OVERLAPPEDEX*> mPool;
	};
}