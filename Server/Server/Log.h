#pragma once

#include <memory>

namespace mk
{
	class Log
	{
	public:
		static void Init();

		static std::shared_ptr<spdlog::logger>& GetLogger() { return sLogger; }

	private:
		static std::shared_ptr<spdlog::logger> sLogger;
	};
}

#ifdef _DEBUG
#define MK_DEBUG
#endif

#define MK_SLOG(...) mk::Log::GetLogger()->info(__VA_ARGS__)

#ifdef MK_DEBUG
#define MK_INFO(...) mk::Log::GetLogger()->info(__VA_ARGS__)
#define MK_WARN(...) mk::Log::GetLogger()->warn(__VA_ARGS__)
#define MK_ERROR(...) mk::Log::GetLogger()->error(__VA_ARGS__)
#define MK_ASSERT(x) {if(!(x)) { __debugbreak();}}
#else
#define MK_INFO(...)
#define MK_WARN(...)
#define MK_ERROR(...)
#define MK_ASSERT(x)
#endif