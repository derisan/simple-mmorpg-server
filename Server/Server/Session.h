#pragma once

#include "Actor.h"
#include "OverlappedEx.h"

namespace mk
{
	constexpr int RECV_BUFFER_SIZE = 1024;
	constexpr int RECV_BUFFER_HALF_SIZE = RECV_BUFFER_SIZE / 2;

	class Session : public Actor
	{
	public:
		Session();

		void BindAccept(SOCKET listenSocket);
		void BindRecv();
		void DoAccept(HANDLE iocp);
		std::vector<char*> DoRecv(const int dataSize);
		void Push(OVERLAPPEDEX* overEX);
		void Disconnect(SOCKET listenSocket);

	public:
		void SendLoginInfoPacket();
		void SendMovePacket(const int id, const unsigned int time);
		void SendAddObjectPacket(const int id);
		void SendRemoveObjectPacket(const int id);
		void SendChatPacket(const int senderID, const char chatType, std::string_view chat);
		void SendStatChangePacket(const int id);

	private:
		OVERLAPPEDEX* pop();
		void sendPacket(void* packet, const int packetSize);

	private:
		std::unique_ptr<OverlappedPool> mPool = nullptr;

		SOCKET mSocket = INVALID_SOCKET;

		OVERLAPPEDEX mAcceptContext = {};

		OVERLAPPEDEX mRecvContext = {};
		char mRecvBuffer[RECV_BUFFER_SIZE] = {};
		int mWritePos = 0;
		int mReadPos = 0;
	};
}
