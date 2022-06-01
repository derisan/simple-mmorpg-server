#include "stdafx.h"
#include "IocpBase.h"

namespace mk
{
	constexpr uint16_t SERVER_PORT = 18354;

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

		MK_INFO("Server initialization success");
		return true;
	}

	void IocpBase::Shutdown()
	{
		WSACleanup();
	}

	void IocpBase::Run()
	{

	}
}
