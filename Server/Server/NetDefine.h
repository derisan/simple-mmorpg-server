#pragma once

#include <concurrent_queue.h>

namespace mk
{
	constexpr uint32_t SEND_BUFFER_SIZE = 200;
	constexpr uint32_t RECV_BUFFER_SIZE = 1024;
	constexpr uint32_t RECV_BUFFER_HALF_SIZE = RECV_BUFFER_SIZE / 2;

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
		void Disconnect();

	public:
		int32_t GetID() const { return mID; }
		void SetID(const int32_t value) { mID = value; }
		std::string GetName() const { return mName; }
		void SetName(std::string_view value) { mName = value; }

		int16_t GetX() const { return mPosX; }
		int16_t GetY() const { return mPosY; }
		std::pair<int16_t, int16_t> GetPos() const { return { mPosX, mPosY }; }
		void SetX(const int16_t value) { mPosX = value; }
		void SetY(const int16_t value) { mPosY = value; }
		void SetPos(const std::pair<int16_t, int16_t>& value) { mPosX = value.first; mPosY = value.second; }

	private:
		int32_t mID = INVALID_VALUE;
		std::string mName = {};
		int16_t mPosX = 0;
		int16_t mPosY = 0;
	};

	class Session : public Actor
	{
	public:
		Session();

		void BindAccept(SOCKET listenSocket);
		void BindRecv();
		void DoAccept(HANDLE iocp);
		std::vector<char*> DoRecv(const int32_t dataSize);
		void Push(OVERLAPPEDEX* overEX);
		void Disconnect(SOCKET listenSocket);

	public:
		void SendLoginInfoPacket();
		void SendMovePacket(const int32_t id, const int32_t x, 
			const int32_t y, const uint32_t time);

	public:
		bool IsConnected() const { return mbConnected; }
		void SetConnect(bool value) { mbConnected = value; }

	private:
		OVERLAPPEDEX* pop();
		void sendPacket(void* packet, const int32_t packetSize);

	private:
		std::unique_ptr<OverlappedPool> mPool = nullptr;

		SOCKET mSocket = INVALID_SOCKET;

		OVERLAPPEDEX mAcceptContext = {};

		OVERLAPPEDEX mRecvContext = {};
		char mRecvBuffer[RECV_BUFFER_SIZE] = {};
		int32_t mWritePos = 0;
		int32_t mReadPos = 0;

		bool mbConnected = false;
	};
}