#pragma once

namespace mk
{
	constexpr uint32_t SEND_BUFFER_SIZE = 200;

	enum class OperationType
	{
		OP_NONE,
		OP_ACCEPT,
		OP_RECV,
		OP_SEND,
	};

	struct OVERLAPPEDEX
	{
		OVERLAPPEDEX()
		{
			WsaBuf.buf = SendBuffer;
			WsaBuf.len = SEND_BUFFER_SIZE;
		}

		WSAOVERLAPPED Overlapped = {};
		WSABUF WsaBuf = {};
		OperationType OpType = OperationType::OP_NONE;
		char SendBuffer[SEND_BUFFER_SIZE] = {};
	};
}