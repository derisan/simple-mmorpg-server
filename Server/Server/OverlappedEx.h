#pragma once

#include <concurrent_queue.h>

namespace mk
{
	constexpr int SEND_BUFFER_SIZE = 256;
	
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
		int ID = INVALID_VALUE;
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