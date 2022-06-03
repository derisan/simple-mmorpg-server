#pragma once

constexpr int NAME_LEN = 20;
constexpr int SERVER_PORT = 10212;

constexpr char CS_LOGIN = 0;
constexpr char SC_LOGIN_INFO = 1;

constexpr char CS_MOVE = 2;
constexpr char SC_MOVE_PLAYER = 3;

#pragma pack(push, 1)

struct CS_LOGIN_PACKET
{
	unsigned char size;
	char type;
	char name[NAME_LEN];
};

struct SC_LOGIN_INFO_PACKET
{
	unsigned char size;
	char type;
	int id;
	short x;
	short y;
	char name[NAME_LEN];
};

struct CS_MOVE_PACKET
{
	unsigned char size;
	char type;
	char direction;  // 0 : UP, 1 : DOWN, 2 : LEFT, 3 : RIGHT
	unsigned int client_time;
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