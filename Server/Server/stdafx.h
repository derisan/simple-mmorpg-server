#pragma once

#define NOT !

#pragma comment(lib, "spdlog")
#pragma comment(lib, "fmt")
#pragma comment(lib, "ws2_32")
#pragma comment(lib, "mswsock")
#pragma comment(lib, "libtcmalloc_minimal")
#pragma comment(linker, "/include:__tcmalloc")

#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

#include <DirectXTK12/SimpleMath.h>
using namespace DirectX::SimpleMath;

#include <Windows.h>
#include <Winsock2.h>
#include <WS2tcpip.h>
#include <MSWSock.h>

#include <memory>
#include <vector>
#include <array>

#include "Log.h"

constexpr int32_t INVALID_VALUE = -1;