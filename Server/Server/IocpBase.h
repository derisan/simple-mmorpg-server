#pragma once

#include <thread>
#include <array>

namespace mk
{
	class Actor;

	constexpr int32_t MAX_USER_NUM = 100;
	constexpr int32_t MAX_NPC_NUM = 100;

	extern std::array<Actor*, MAX_USER_NUM + MAX_NPC_NUM> gClients;

	class IocpBase
	{
	public:
		bool Init();

		void Shutdown();

		void Run();

	private:
		void doWorker();

		void disconnect(const int32_t id);

	private:
		SOCKET mListenSocket = INVALID_SOCKET;
		HANDLE mIocp = INVALID_HANDLE_VALUE;

		std::vector<std::thread> mWorkerThreads;
		std::thread mTimerThread;
	};
}
