#pragma once

#include "CommonDefine.h"

constexpr char CS_LOGIN = 0;
constexpr char CS_MOVE = 1;

constexpr char SC_LOGIN_INFO = 2;
constexpr char SC_ADD_PLAYER = 3;
constexpr char SC_REMOVE_PLAYER = 4;
constexpr char SC_MOVE_PLAYER = 5;
constexpr char SC_CHAT = 6;

#pragma pack(push, 1)

struct CS_LOGIN_PACKET
{
	unsigned char size;
	char type;
	char name[NAME_LEN];
};

struct CS_MOVE_PACKET
{
	unsigned char size;
	char type;
	char direction;  // 0 : UP, 1 : DOWN, 2 : LEFT, 3 : RIGHT
	unsigned int client_time;
};

struct SC_LOGIN_INFO_PACKET
{
	unsigned char size;
	char type;
	int id;
	short x;
	short y;
};

struct SC_MOVE_PLAYER_PACKET
{
	unsigned char size;
	char type;
	int	id;
	short x;
	short y;
	unsigned int client_time;
};

#pragma pack(pop)