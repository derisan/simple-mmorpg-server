#include "stdafx.h"
#include "IocpBase.h"

#include "NetDefine.h"

namespace mk
{
	constexpr uint16_t SERVER_PORT = 9999;
	constexpr int32_t WORKER_THREAD_NUM = 6;

	std::array<Actor*, MAX_USER_NUM + MAX_NPC_NUM> gClients;

	bool IocpBase::Init()
	{
		WSADATA wsaData = {};
		int32_t ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
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
		serverAddr.sin_port = htons(SERVER_PORT);
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

		for (auto i = 0; i < WORKER_THREAD_NUM; ++i)
		{
			mWorkerThreads.emplace_back([this]() { doWorker(); });
		}

		for (auto idx = 0; idx < MAX_USER_NUM; ++idx)
		{
			auto session = new Session;
			session->SetID(idx);
			session->BindAccept(mListenSocket);
			gClients[idx] = session;
		}

		MK_INFO("Server initialization success");
		return true;
	}

	void IocpBase::Shutdown()
	{
		WSACleanup();
	}

	void IocpBase::Run()
	{
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
			int32_t id = static_cast<int32_t>(key);

			if (FALSE == ret)
			{
				if (OperationType::OP_ACCEPT == overEx->OpType)
				{
					MK_ERROR("Accept error.");
				}
				else
				{
					MK_ERROR("GQCS error on client: {0}", id);
					disconnect(id);
					if (OperationType::OP_SEND == overEx->OpType)
					{
						static_cast<Session*>(gClients[id])->Push(overEx);
					}
					continue;
				}
			}

			switch (overEx->OpType)
			{
			case OperationType::OP_ACCEPT:
			{
				MK_INFO("Client[{0}] connected.", overEx->ID);
				auto session = static_cast<Session*>(gClients[overEx->ID]);
				session->DoAccept(mIocp);
			}
				break;

			case OperationType::OP_RECV:
			{
				if (0 == numBytes)
				{
					disconnect(id);
				}
				else
				{
					overEx->SendBuffer[numBytes] = '\0';
					MK_INFO("Client[{0}] sent: {1}", id, overEx->SendBuffer);
					auto session = static_cast<Session*>(gClients[id]);
					session->EchoTest(overEx->SendBuffer, numBytes);
					session->BindRecv();
				}
			}
				break;

			case OperationType::OP_SEND:
			{
				if (0 == numBytes)
				{
					disconnect(id);
				}
				static_cast<Session*>(gClients[id])->Push(overEx);
			}
				break;

			default:
				MK_ASSERT(false);
				break;
			}
		}
	}

	void IocpBase::disconnect(int32_t id)
	{
		MK_INFO("Client[{0}] disconnected.", id);
		static_cast<Session*>(gClients[id])->Disconnect(mListenSocket);
	}
}
