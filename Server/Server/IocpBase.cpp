#include "stdafx.h"
#include "IocpBase.h"

#include "Session.h"
#include "Timer.h"
#include "SectorManager.h"
#include "NPC.h"
#include "Random.h"
#include "DBConnection.h"

namespace mk
{
	std::array<Actor*, MAX_USER + NUM_NPC> gClients;

	inline Session* GetSession(const id_t id)
	{
		return static_cast<Session*>(gClients[id]);
	}

	inline int GetLastActorIndex()
	{
		for (int idx = 0; idx < MAX_USER + NUM_NPC; ++idx)
		{
			if (NOT gClients[idx])
			{
				return idx;
			}
		}

		return -1;
	}

	bool IocpBase::Init()
	{
		Random::Init();

		WSADATA wsaData = {};
		int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (0 != ret)
		{
			MK_ERROR("WSAStartup failed");
			return false;
		}

		mListenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
		if (INVALID_SOCKET == mListenSocket)
		{
			MK_ERROR("Failed to create listen socket: {0}", WSAGetLastError());
			return false;
		}

		BOOL opt = true;
		ret = setsockopt(mListenSocket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&opt), sizeof(opt));
		if (SOCKET_ERROR == ret)
		{
			MK_ERROR("Failed to set socket option: {0}", WSAGetLastError());
			return false;
		}

		SOCKADDR_IN serverAddr = {};
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = htons(PORT_NUM);
		serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
		ret = bind(mListenSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr));
		if (SOCKET_ERROR == ret)
		{
			MK_ERROR("Failed to bind: {0}", WSAGetLastError());
			return false;
		}

		ret = listen(mListenSocket, SOMAXCONN);
		if (SOCKET_ERROR == ret)
		{
			MK_ERROR("Failed to listen: {0}", WSAGetLastError());
			return false;
		}

		mIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
		if (NULL == mIocp)
		{
			MK_ERROR("Failed to create iocp: {0}", GetLastError());
			return false;
		}

		HANDLE iocp = CreateIoCompletionPort(reinterpret_cast<HANDLE>(mListenSocket), mIocp, 0, 0);
		if (iocp != mIocp)
		{
			MK_ERROR("Failed to associate listen socket to iocp: {0}", GetLastError());
			return false;
		}

		auto workerThreadNum = static_cast<int>(std::thread::hardware_concurrency() * 1.5);
		for (auto i = 0; i < workerThreadNum; ++i)
		{
			mWorkerThreads.emplace_back([this]() { doWorker(); });
		}

		Timer::Init(mIocp);
		DBConnection::Init(mIocp);

		mTimerThread = std::thread{ []() { Timer::Run(); } };
		mDBThread = std::thread{ []() {DBConnection::Run(); } };

		SectorManager::Init();

		for (int idx = 0; idx < MAX_USER; ++idx)
		{
			auto session = new Session;
			session->SetID(idx);
			session->BindAccept(mListenSocket);
			gClients[idx] = session;
		}

		// Session과 NPC 생성이 선행되어야 함.
		mTickThread = std::thread{ [this]() { doTick(); } };

		auto lastIndex = GetLastActorIndex();
		mAIThread = std::thread{ [this, lastIndex] { doAI(lastIndex); } };

		MK_SLOG("Server initialization success");
		return true;
	}

	void IocpBase::Shutdown()
	{
		DBConnection::Shutdown();

		WSACleanup();
	}

	void IocpBase::Run()
	{
		if (mTickThread.joinable())
		{
			mTickThread.join();
		}

		if (mTimerThread.joinable())
		{
			mTimerThread.join();
		}

		for (auto& th : mWorkerThreads)
		{
			if (th.joinable())
			{
				th.join();
			}
		}
	}

	void IocpBase::doWorker()
	{
		while (true)
		{
			DWORD numBytes = 0;
			ULONG_PTR key = 0;
			OVERLAPPEDEX* overEx = nullptr;

			BOOL ret = GetQueuedCompletionStatus(mIocp, &numBytes, &key, (LPOVERLAPPED*)&overEx, INFINITE);
			id_t id = static_cast<id_t>(key);

			if (FALSE == ret)
			{
				if (OperationType::OP_ACCEPT == overEx->OpType)
				{
					MK_ERROR("Accept error.");
					auto session = GetSession(overEx->ID);
					session->BindAccept(mListenSocket);
				}
				else
				{
					MK_ERROR("GQCS error on client: {0}", id);
					disconnect(id);
					if (OperationType::OP_SEND == overEx->OpType)
					{
						auto session = GetSession(id);
						session->Push(overEx);
					}
				}
				continue;
			}

			switch (overEx->OpType)
			{
			case OperationType::OP_ACCEPT:
			{
				auto session = GetSession(overEx->ID);
				session->DoAccept(mIocp);
				break;
			}

			case OperationType::OP_RECV:
			{
				if (0 == numBytes)
				{
					disconnect(id);
				}
				else
				{
					auto session = GetSession(id);
					std::vector<char*> packets = session->DoRecv(numBytes);
					for (auto p : packets)
					{
						processPacket(id, p);
					}
				}
				break;
			}

			case OperationType::OP_SEND:
			{
				if (0 == numBytes)
				{
					disconnect(id);
				}
				auto session = GetSession(id);
				session->Push(overEx);
				break;
			}

			case OperationType::TIMER_BIND_ACCEPT:
			{
				auto session = GetSession(id);
				session->BindAccept(mListenSocket);
				Timer::PushOverEx(overEx);
				break;
			}

			case OperationType::TIMER_RESET_ATTACK:
			{
				auto session = GetSession(id);
				session->SetAttack(true);
				Timer::PushOverEx(overEx);
				break;
			}

			case OperationType::DB_LOGIN_NO_INFO:
			{
				auto session = GetSession(id);

				auto [x, y] = SectorManager::GetAvailablePos();
				session->SetPos(x, y);
				auto level = 1;
				session->SetLevel(level);
				session->SetMaxHP(level * 100);
				session->SetCurrentHP(level * 100);
				session->SetRace(Race::Player);
				session->SetAttackPower(level);
				session->SetExp(0);
				session->SetRequiredExp(level * 10);
				session->SetActive(true);
				session->SendLoginInfoPacket();
				SectorManager::AddActor(session);
				DBConnection::PushOverEx(overEx);
				break;
			}

			case OperationType::DB_LOGIN_WITH_INFO:
			{
				UserInfo info = {};
				CopyMemory(&info, overEx->SendBuffer, sizeof(info));

				auto session = GetSession(id);
				session->SetPos(info.X, info.Y);
				session->SetLevel(info.Level);
				session->SetMaxHP((info.Level - 1) * 20 + 100);
				session->SetCurrentHP(info.HP);
				session->SetRace(info.Race);
				auto attackPower = info.Level;
				session->SetAttackPower(attackPower);
				session->SetExp(info.Exp);
				auto requiredExp = info.Level * 10;
				session->SetRequiredExp(requiredExp);
				session->SetActive(true);
				session->SendLoginInfoPacket();
				SectorManager::AddActor(session);
				DBConnection::PushOverEx(overEx);
				break;
			}

			default:
				MK_ASSERT(false);
				break;
			}
		}
	}

	void IocpBase::doTick()
	{
		using namespace std::chrono;

		while (true)
		{
			auto start = system_clock::now();

			for (auto idx = 0; idx < MAX_USER; ++idx)
			{
				gClients[idx]->Tick();
			}

			std::this_thread::sleep_until(start + 5s);
		}
	}

	void IocpBase::doAI(const int lastIndex)
	{
		using namespace std::chrono;

		while (true)
		{
			auto start = system_clock::now();

			for (auto idx = MAX_USER; idx < lastIndex; ++idx)
			{
				gClients[idx]->Tick();
			}

			std::this_thread::sleep_until(start + 1s);
		}
	}

	void IocpBase::disconnect(const id_t id)
	{
		MK_INFO("Client[{0}] disconnected.", id);
		auto session = GetSession(id);
		SectorManager::RemoveActor(session);
		session->Shutdown();
	}

	void IocpBase::processPacket(const id_t id, char* packet)
	{
		char packetType = packet[1];

		switch (packetType)
		{
		case CS_LOGIN:
		{
			CS_LOGIN_PACKET* loginPacket = reinterpret_cast<CS_LOGIN_PACKET*>(packet);
			auto session = GetSession(id);
			session->SetName(loginPacket->name);
			DBConnection::PushJob(id, DBJobType::GetUserInfo);
			break;
		}

		case CS_MOVE:
		{
			CS_MOVE_PACKET* movePacket = reinterpret_cast<CS_MOVE_PACKET*>(packet);
			auto session = GetSession(id);
			SectorManager::MoveActor(session, movePacket->direction,
				movePacket->client_time);
			break;
		}

		case CS_ATTACK:
		{
			CS_ATTACK_PACKET* attackPacket = reinterpret_cast<CS_ATTACK_PACKET*>(packet);
			auto session = GetSession(id);
			bool canAttack = false;
			{
				ReadLockGuard guard = { session->ActorLock };
				canAttack = session->CanAttack();
			}
			if (canAttack)
			{
				SectorManager::DoAttack(session);
			}
			break;
		}

		case CS_CHAT:
		{
			CS_CHAT_PACKET* chatPacket = reinterpret_cast<CS_CHAT_PACKET*>(packet);
			auto session = GetSession(id);
			SectorManager::SendChat(session, chatPacket->chat_type, chatPacket->mess);
			break;
		}

		default:
			break;
		}
	}
}
