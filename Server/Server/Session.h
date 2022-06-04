#pragma once

#include "Actor.h"
#include "OverlappedEx.h"

namespace mk
{
	constexpr uint32_t RECV_BUFFER_SIZE = 1024;
	constexpr uint32_t RECV_BUFFER_HALF_SIZE = RECV_BUFFER_SIZE / 2;

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
