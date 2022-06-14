# pragma once
//# define NO_S3D_USING
#include <Siv3D.hpp>
#include "../../Server/Server/Protocol.h"

#define MK_ASSERT(x) {if(!(x)) { __debugbreak(); }}

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
