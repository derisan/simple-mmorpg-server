#include "stdafx.h"
#include "Session.h"

#include "Protocol.h"
#include "Timer.h"
#include "IocpBase.h"

namespace mk
{
	using namespace std::chrono;

	Session::Session()
	{
		mPool = std::make_unique<OverlappedPool>();
	}

	void Session::BindAccept(SOCKET listenSocket)
	{
		mSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

		if (INVALID_SOCKET == mSocket)
		{
			MK_ERROR("Failed to create socket: {0}", WSAGetLastError());
			return;
		}

		ZeroMemory(&mAcceptContext, sizeof(mAcceptContext));
		mAcceptContext.OpType = OperationType::OP_ACCEPT;
		mAcceptContext.ID = GetID();

		BOOL ret = AcceptEx(listenSocket,
			mSocket,
			mAcceptContext.SendBuffer,
			0,
			sizeof(SOCKADDR_IN) + 16,
			sizeof(SOCKADDR_IN) + 16,
			0,
			&mAcceptContext.Overlapped);

		if (FALSE == ret && (WSAGetLastError() != WSA_IO_PENDING))
		{
			MK_ERROR("AcceptEx error: {0}", WSAGetLastError());
			return;
		}
	}

	void Session::DoAccept(HANDLE iocp)
	{
		HANDLE hIocp = CreateIoCompletionPort(reinterpret_cast<HANDLE>(mSocket),
			iocp,
			GetID(),
			0);

		if (hIocp != iocp)
		{
			MK_ERROR("Failed to associate client socket: {0}", GetLastError());
			return;
		}

		SetConnect(true);
		BindRecv();
	}

	std::vector<char*> Session::DoRecv(const int dataSize)
	{
		std::vector<char*> packets;

		mWritePos += dataSize;
		auto remain = mWritePos - mReadPos;
		while (remain > 0)
		{
			auto packetSize = static_cast<unsigned char>(mRecvBuffer[mReadPos]);
			if (remain >= packetSize)
			{
				packets.push_back(&mRecvBuffer[mReadPos]);
				remain -= packetSize;
				mReadPos += packetSize;
			}
			else
			{
				break;
			}
		}

		if (mWritePos >= RECV_BUFFER_HALF_SIZE)
		{
			CopyMemory(&mRecvBuffer[0], &mRecvBuffer[mReadPos], remain);
			mReadPos = 0;
			mWritePos = remain;
		}

		BindRecv();

		return packets;
	}

	void Session::BindRecv()
	{
		ZeroMemory(&mRecvContext.Overlapped, sizeof(mRecvContext.Overlapped));
		mRecvContext.OpType = OperationType::OP_RECV;
		mRecvContext.WsaBuf.buf = mRecvBuffer + mWritePos;
		mRecvContext.WsaBuf.len = RECV_BUFFER_HALF_SIZE;

		DWORD flags = 0;
		int ret = WSARecv(mSocket,
			&mRecvContext.WsaBuf,
			1,
			NULL,
			&flags,
			&mRecvContext.Overlapped,
			NULL);

		if (SOCKET_ERROR == ret && (WSAGetLastError() != WSA_IO_PENDING))
		{
			MK_ERROR("WSARecv failed: {0}", WSAGetLastError());
			return;
		}
	}

	void Session::Disconnect(SOCKET listenSocket)
	{
		Actor::Disconnect();

		shutdown(mSocket, SD_BOTH);
		closesocket(mSocket);
		mSocket = INVALID_SOCKET;
		mWritePos = 0;
		mReadPos = 0;
		SetConnect(false);
		Timer::AddEvent(TimerEventType::EV_BIND_ACCEPT, GetID(), system_clock::now() + 3s);
	}

	OVERLAPPEDEX* Session::pop()
	{
		return mPool->Pop();
	}

	void Session::Push(OVERLAPPEDEX* overEX)
	{
		mPool->Push(overEX);
	}

	void Session::SendLoginInfoPacket()
	{
		SC_LOGIN_OK_PACKET packet = {};
		packet.size = sizeof(packet);
		packet.type = SC_LOGIN_OK;
		packet.id = GetID();
		packet.race = GetRace();
		packet.x = GetX();
		packet.y = GetY();
		packet.level = GetLevel();
		packet.exp = 0; // TODO : Exp
		packet.hp = GetCurrentHP();
		packet.hpmax = GetMaxHP();
		sendPacket(&packet, packet.size);
	}

	void Session::SendMovePacket(const int id, const short x, const short y,
		const unsigned int time)
	{
		SC_MOVE_OBJECT_PACKET packet = {};
		packet.size = sizeof(packet);
		packet.type = SC_MOVE_OBJECT;
		packet.id = id;
		packet.x = x;
		packet.y = y;
		packet.client_time = time;
		sendPacket(&packet, packet.size);
	}

	void Session::SendAddObjectPacket(const int id, const short x, const short y, 
		const std::string& name)
	{
		SC_ADD_OBJECT_PACKET packet = {};
		packet.size = sizeof(packet);
		packet.type = SC_ADD_OBJECT;
		packet.id = id;
		packet.x = x;
		packet.y = y;
		CopyMemory(packet.name, name.data(), name.length());
		sendPacket(&packet, packet.size);
	}

	void Session::SendRemoveObjectPacket(const int id)
	{
		SC_REMOVE_OBJECT_PACKET packet = {};
		packet.size = sizeof(packet);
		packet.type = SC_REMOVE_OBJECT;
		packet.id = id;
		sendPacket(&packet, packet.size);
	}

	void Session::SendChatPacket(const int senderID, const char chatType, std::string_view chat)
	{
		SC_CHAT_PACKET packet = {};
		packet.size = sizeof(packet);
		packet.type = SC_CHAT;
		packet.chat_type = chatType;
		packet.id = senderID;
		CopyMemory(packet.mess, chat.data(), chat.length());		
		sendPacket(&packet, packet.size);
	}

	void Session::sendPacket(void* packet, const int packetSize)
	{
		OVERLAPPEDEX* overEx = mPool->Pop();
		ZeroMemory(overEx, sizeof(OVERLAPPEDEX));
		overEx->ID = GetID();
		overEx->OpType = OperationType::OP_SEND;
		CopyMemory(overEx->SendBuffer, reinterpret_cast<char*>(packet), packetSize);
		overEx->WsaBuf.buf = overEx->SendBuffer;
		overEx->WsaBuf.len = packetSize;

		int ret = WSASend(mSocket,
			&overEx->WsaBuf,
			1,
			NULL,
			0,
			&overEx->Overlapped,
			NULL);

		if (SOCKET_ERROR == ret && (WSAGetLastError() != WSA_IO_PENDING))
		{
			mPool->Push(overEx);
			return;
		}
	}
}