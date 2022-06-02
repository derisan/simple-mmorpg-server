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

	class Actor
	{	
	public:
		int32_t GetID() const { return mID; }
		void SetID(const int32_t value) { mID = value; }

	private:
		int32_t mID = INVALID_VALUE;
	};

	class Session : public Actor
	{
	public:
		Session();

		void BindAccept(SOCKET listenSocket);
		void BindRecv();
		void DoAccept(HANDLE iocp);
		void Disconnect(SOCKET listenSocket);
		void EchoTest(const char* msg, int32_t msgSize);

		OVERLAPPEDEX* Pop();

		void Push(OVERLAPPEDEX* overEX);

	public:
		bool IsConnected() const { return mbConnected; }
		void SetConnect(bool value) { mbConnected = value; }

	private:
		std::unique_ptr<OverlappedPool> mPool = nullptr;

		SOCKET mSocket = INVALID_SOCKET;

		OVERLAPPEDEX mAcceptContext = {};

		OVERLAPPEDEX mRecvContext = {};

		bool mbConnected = false;
	};
}