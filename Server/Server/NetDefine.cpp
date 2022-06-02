#include "stdafx.h"
#include "NetDefine.h"

namespace mk
{
	constexpr uint32_t MAX_OVEREX_NUM = 100;

	OverlappedPool::OverlappedPool()
	{
		for (uint32_t i = 0; i < MAX_OVEREX_NUM; ++i)
		{
			auto overEX = new OVERLAPPEDEX{};
			mPool.push(overEX);
		}
	}

	OverlappedPool::~OverlappedPool()
	{
		OVERLAPPEDEX* overEX = nullptr;
		while (mPool.try_pop(overEX))
		{
			delete overEX;
			overEX = nullptr;
		}
		mPool.clear();
	}

	mk::OVERLAPPEDEX* OverlappedPool::Pop()
	{
		OVERLAPPEDEX* overEX = nullptr;
		if (NOT mPool.try_pop(overEX))
		{
			overEX = new OVERLAPPEDEX{};
		}
		return overEX;
	}

	void OverlappedPool::Push(OVERLAPPEDEX* overEX)
	{
		mPool.push(overEX);
	}

	Session::Session()
	{
		mPool = std::make_unique<OverlappedPool>();
	}

	void Session::BindAccept(SOCKET listenSocket)
	{
		MK_INFO("Client[{0}] BindAccept()", GetID());

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

	void Session::BindRecv()
	{
		ZeroMemory(&mRecvContext, sizeof(mRecvContext));
		mRecvContext.OpType = OperationType::OP_RECV;
		mRecvContext.WsaBuf.buf = mRecvContext.SendBuffer;
		mRecvContext.WsaBuf.len = SEND_BUFFER_SIZE;

		DWORD flags = 0;
		int32_t ret = WSARecv(mSocket,
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
		closesocket(mSocket);
		mSocket = INVALID_SOCKET;
		SetConnect(false);
		BindAccept(listenSocket);
	}

	void Session::EchoTest(const char* msg, int32_t msgSize)
	{
		OVERLAPPEDEX* overEx = mPool->Pop();
		ZeroMemory(overEx, sizeof(overEx));
		overEx->ID = GetID();
		overEx->OpType = OperationType::OP_SEND;
		CopyMemory(overEx->SendBuffer, msg, msgSize);
		overEx->WsaBuf.buf = overEx->SendBuffer;
		overEx->WsaBuf.len = msgSize;

		int32_t ret = WSASend(mSocket,
			&overEx->WsaBuf,
			1,
			NULL,
			0,
			&overEx->Overlapped,
			NULL);

		if (SOCKET_ERROR == ret && (WSAGetLastError() != WSA_IO_PENDING))
		{
			mPool->Push(overEx);
			MK_ERROR("WSASend failed: {0}", WSAGetLastError());
			return;
		}
	}

	OVERLAPPEDEX* Session::Pop()
	{
		return mPool->Pop();
	}

	void Session::Push(OVERLAPPEDEX* overEX)
	{
		mPool->Push(overEX);
	}
}
