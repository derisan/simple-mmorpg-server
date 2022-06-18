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

		void doTick();

		void doAI(const int lastIndex);

		void disconnect(const id_t id);

		void processPacket(const id_t id, char* packet);

	private:
		SOCKET mListenSocket = INVALID_SOCKET;
		HANDLE mIocp = INVALID_HANDLE_VALUE;

		std::vector<std::thread> mWorkerThreads;
		std::thread mTimerThread;
		std::thread mTickThread;
		std::thread mAIThread;
		std::thread mDBThread;
	};
}
