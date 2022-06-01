#pragma once

namespace mk
{
	class IocpBase
	{
	public:
		bool Init();

		void Shutdown();

		void Run();

	private:
		SOCKET mListenSocket = INVALID_SOCKET;
		HANDLE mIocp = INVALID_HANDLE_VALUE;
	};
}
