#pragma once

#include <thread>
#include <array>

#include "Protocol.h"

namespace mk
{
	class Actor;

	extern std::array<Actor*, MAX_USER + NUM_NPC> gClients;

	class IocpBase
	{
	public:
		bool Init();

		void Shutdown();

		void Run();

	private:
		void doWorker();

		void disconnect(const int id);

		void processPacket(const int id, char* packet);

	private:
		SOCKET mListenSocket = INVALID_SOCKET;
		HANDLE mIocp = INVALID_HANDLE_VALUE;

		std::vector<std::thread> mWorkerThreads;
		std::thread mTimerThread;
	};
}
