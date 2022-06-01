#include "stdafx.h"
#include "Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace mk
{
	std::shared_ptr<spdlog::logger> Log::sLogger = nullptr;

	void Log::Init()
	{
		spdlog::set_pattern("%^[%T] %n: %v%$");
		sLogger = spdlog::stdout_color_mt("Server");
		sLogger->set_level(spdlog::level::trace);
	}
}
