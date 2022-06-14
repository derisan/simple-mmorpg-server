#pragma once

class PacketManager
{
public:
	static void Recv();
	static void RegisterPacketFunc(char packetType, std::function<void(char*)> func);
	static void RemovePacketFunc(char packetType);

private:
	constexpr static int32 RECV_BUFFER_SIZE = 200;
	constexpr static int32 PACKET_BUFFER_SIZE = 2048;

	static char mRecvBuffer[RECV_BUFFER_SIZE];
	static char mPacketBuffer[PACKET_BUFFER_SIZE];
	static int32 mWritePos;
	static int32 mReadPos;

	static std::unordered_map<char, std::function<void(char*)>> mPacketFuncDict;
};

