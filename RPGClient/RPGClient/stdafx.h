# pragma once
//# define NO_S3D_USING
#include <Siv3D.hpp>
#include "../../Server/Server/Protocol.h"

#define MK_ERROR(x, error) {if(!(x)) { throw Error{ error }; }}

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#define ASSET_PATH(x) U"Assets/"#x
