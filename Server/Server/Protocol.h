#pragma once

constexpr int PORT_NUM = 4000;
constexpr int BUF_SIZE = 200;
constexpr int NAME_SIZE = 20;

constexpr int W_WIDTH = 2000;
constexpr int W_HEIGHT = 2000;

constexpr int MAX_USER = 3000;
constexpr int NUM_NPC = 200000;

// Packet ID
constexpr char CS_LOGIN = 0;
constexpr char CS_MOVE = 1;
constexpr char CS_ATTACK = 2;
constexpr char CS_CHAT = 3;

constexpr char SC_LOGIN_OK = 11;
constexpr char SC_LOGIN_FAIL = 12;
constexpr char SC_ADD_OBJECT = 13;
constexpr char SC_REMOVE_OBJECT = 14;
constexpr char SC_MOVE_OBJECT = 15;
constexpr char SC_CHAT = 16;
constexpr char SC_STAT_CHANGE = 17;

#pragma pack(push, 1)

struct CS_LOGIN_PACKET
{
	unsigned char size;
	char type;
	char name[NAME_SIZE];
};

struct CS_MOVE_PACKET
{
	unsigned char size;
	char type;
	char direction;  // 0 : UP, 1 : DOWN, 2 : LEFT, 3 : RIGHT
	unsigned int client_time;
};

struct SC_LOGIN_OK_PACKET
{
	unsigned char size;
	char type;
	int id;
	short x;
	short y;
};

struct SC_LOGIN_FAIL_PACKET {
	unsigned char size;
	char	type;
	int		reason;				// 0 : Invalid Name  (특수문자, 공백 제외)
								// 1 : Name Already Playing
								// 2 : Server Full
};

struct SC_ADD_OBJECT_PACKET {
	unsigned char size;
	char	type;
	int		id;
	short	x, y;
	//short	race;			// 종족 : 인간, 엘프, 드워프, 오크, 드래곤
							// 클라이언트에서 종족별로 별도의 그래픽 표현
							// 추가적으로 성별이나, 직업을 추가할 수 있다.
	char	name[NAME_SIZE];
	//short	level;
	//int		hp, hpmax;
};

struct SC_REMOVE_OBJECT_PACKET {
	unsigned char size;
	char	type;
	int	id;
};

struct SC_MOVE_OBJECT_PACKET
{
	unsigned char size;
	char type;
	int	id;
	short x;
	short y;
	unsigned int client_time;
};

#pragma pack(pop)