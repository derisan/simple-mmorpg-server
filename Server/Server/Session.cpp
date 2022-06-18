#include "stdafx.h"
#include "Session.h"

#include "Protocol.h"
#include "Timer.h"
#include "IocpBase.h"
#include "SectorManager.h"
#include "DBConnection.h"

namespace mk
{
	using namespace std::chrono;

	constexpr int SYSTEM_CHAT_ID = -1;

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

	void Session::Shutdown()
	{
		Actor::Shutdown();

		shutdown(mSocket, SD_BOTH);
		closesocket(mSocket);
		mSocket = INVALID_SOCKET;
		mWritePos = 0;
		mReadPos = 0;
		mRequiredExp = INT_MAX;
		Timer::AddEvent(TimerEventType::EV_BIND_ACCEPT, GetID(), system_clock::now() + 3s);
	}

	void Session::Tick()
	{
		if (NOT IsActive())
		{
			return;
		}

		auto maxHP = GetMaxHP();
		auto regen = maxHP / 10;

		int currentHP = 0;
		int newHP = 0;
		{
			WriteLockGuard guard = { ActorLock };
			currentHP = GetCurrentHP();
			newHP = std::clamp(currentHP + regen,
				currentHP, maxHP);
			SetCurrentHP(newHP);
		}

		if (currentHP != newHP)
		{
			SectorManager::SendStatChangeToViewList(this);
		}

		DBConnection::PushJob(GetID(), DBJobType::UpdateUserInfo);
	}

	bool Session::AddToViewList(const id_t id, const bool bSendMove /*= false*/)
	{
		bool bInsert = false;

		{
			WriteLockGuard guard = { ViewLock };
			auto [_, bResult] = ViewList.insert(id);
			bInsert = bResult;
		}

		if (bInsert)
		{
			SendAddObjectPacket(id);
		}
		else
		{
			if (bSendMove)
			{
				SendMovePacket(id, 0);
			}
		}

		return bInsert;
	}

	bool Session::RemoveFromViewList(const id_t id)
	{
		size_t cnt = 0;

		{
			WriteLockGuard guard = { ViewLock };
			cnt = ViewList.erase(id);
		}

		if (0 != cnt)
		{
			SendRemoveObjectPacket(id);
			return true;
		}

		return false;
	}

	bool Session::OnKillEnemy(const int incomingExp)
	{
		auto exp = GetExp() + incomingExp;

		bool bLevelUp = false;

		if (exp >= mRequiredExp)
		{
			auto level = GetLevel() + 1;
			SetLevel(level);
			exp -= mRequiredExp;
			auto maxHP = GetMaxHP() + 20;
			SetMaxHP(maxHP);
			SetCurrentHP(maxHP);
			SetAttackPower(level);
			SetRequiredExp(level * 10);
			bLevelUp = true;
		}

		SetExp(exp);

		return bLevelUp;
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
		packet.exp = GetExp();
		packet.hp = GetCurrentHP();
		packet.hpmax = GetMaxHP();
		sendPacket(&packet, packet.size);
	}

	void Session::SendMovePacket(const id_t id, const unsigned int time)
	{
		SC_MOVE_OBJECT_PACKET packet = {};
		packet.size = sizeof(packet);
		packet.type = SC_MOVE_OBJECT;
		packet.id = id;
		packet.x = gClients[id]->GetX();
		packet.y = gClients[id]->GetY();
		packet.client_time = time;
		sendPacket(&packet, packet.size);
	}

	void Session::SendAddObjectPacket(const id_t id)
	{
		SC_ADD_OBJECT_PACKET packet = {};
		packet.size = sizeof(packet);
		packet.type = SC_ADD_OBJECT;
		packet.id = id;
		packet.x = gClients[id]->GetX();
		packet.y = gClients[id]->GetY();
		packet.race = gClients[id]->GetRace();
		packet.level = gClients[id]->GetLevel();
		packet.hp = gClients[id]->GetCurrentHP();
		packet.hpmax = gClients[id]->GetMaxHP();
		const auto& name = gClients[id]->GetName();
		CopyMemory(packet.name, name.data(), name.length());
		sendPacket(&packet, packet.size);
	}

	void Session::SendRemoveObjectPacket(const id_t id)
	{
		SC_REMOVE_OBJECT_PACKET packet = {};
		packet.size = sizeof(packet);
		packet.type = SC_REMOVE_OBJECT;
		packet.id = id;
		sendPacket(&packet, packet.size);
	}

	void Session::SendSystemChatDamage(const id_t victimID)
	{
		const auto& victimName = gClients[victimID]->GetName();
		const auto myPower = GetAttackPower();

		SendChatPacket(SYSTEM_CHAT_ID,
			0,
			victimName + "을(를) 때려 " + std::to_string(myPower)
			+ "의 데미지를 입혔습니다.");
	}

	void Session::SendSystemChatExp(const id_t victimID)
	{
		const auto& victimName = gClients[victimID]->GetName();
		const auto victimExp = gClients[victimID]->GetExp();

		SendChatPacket(SYSTEM_CHAT_ID,
			0,
			victimName + "을(를) 무찔러 " + std::to_string(victimExp)
			+ "의 경험치를 얻었습니다.");
	}

	void Session::SendChatPacket(const id_t senderID, const char chatType, std::string_view chat)
	{
		SC_CHAT_PACKET packet = {};
		packet.size = sizeof(packet);
		packet.type = SC_CHAT;
		packet.chat_type = chatType;
		packet.id = senderID;
		CopyMemory(packet.mess, chat.data(), chat.length());
		sendPacket(&packet, packet.size);
	}

	void Session::SendStatChangePacket(const id_t id)
	{
		SC_STAT_CHANGE_PACKET packet = {};
		packet.size = sizeof(packet);
		packet.type = SC_STAT_CHANGE;
		packet.id = id;
		packet.level = gClients[id]->GetLevel();
		packet.exp = gClients[id]->GetExp();
		packet.hp = gClients[id]->GetCurrentHP();
		packet.hpmax = gClients[id]->GetMaxHP();
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