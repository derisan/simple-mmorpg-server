#pragma once

#define NOT !

#pragma comment(lib, "spdlog")
#pragma comment(lib, "fmt")
#pragma comment(lib, "ws2_32")
#pragma comment(lib, "mswsock")
#pragma comment(lib, "libtcmalloc_minimal")
#pragma comment(linker, "/include:__tcmalloc")
#pragma comment (lib, "lua54.lib")

#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

#include <Windows.h>
#include <Winsock2.h>
#include <WS2tcpip.h>
#include <MSWSock.h>

#include <memory>
#include <vector>
#include <array>

#include "Log.h"

extern "C"
{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

constexpr int INVALID_VALUE = -1;

using vec2 = POINTS;
using id_t = int;