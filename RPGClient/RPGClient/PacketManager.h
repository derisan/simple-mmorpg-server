#pragma once

class PacketManager
{
public:
	static void Init();
	static void Shutdown();

	static void Recv();
	static bool Connect();
	static void Disconnect();
	static void RegisterPacketFunc(char packetType, std::function<void(char*)> func);
	static void RemovePacketFunc(char packetType);

	static void SendPacket(void* packet, const int32 packetSize);

private:
	constexpr static int32 RECV_BUFFER_SIZE = 200;
	constexpr static int32 PACKET_BUFFER_SIZE = 2048;

	static char sRecvBuffer[RECV_BUFFER_SIZE];
	static char sPacketBuffer[PACKET_BUFFER_SIZE];
	static int32 sWritePos;
	static int32 sReadPos;

	static TCPClient sTCPContext;

	static String sIP;

	static std::unordered_map<char, std::function<void(char*)>> sPacketFuncDict;
};

