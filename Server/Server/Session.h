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

		virtual void Shutdown() override;
		virtual void Tick() override;
		virtual bool AddToViewList(const id_t id, const bool bSendMove = false) override;
		virtual bool RemoveFromViewList(const id_t id) override;
		
		void OnKillEnemy(const int incomingExp);
		void OnHit(const id_t hitterID);

		void SetRequiredExp(const int value) { mRequiredExp = value; }

	public:
		void SendLoginInfoPacket();
		void SendMovePacket(const id_t id, const unsigned int time);
		void SendAddObjectPacket(const id_t id);
		void SendRemoveObjectPacket(const id_t id);
		void SendSystemChatDamage(const id_t victimID);
		void SendSystemChatExp(const id_t victimID);
		void SendSystemChatTakeDamage(const id_t hitterID);
		void SendSystemChatDie(const id_t hitterID);
		void SendChatPacket(const id_t senderID, const char chatType, std::string_view chat);
		void SendStatChangePacket(const id_t id);

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

		int mRequiredExp = INT_MAX;
	};
}
