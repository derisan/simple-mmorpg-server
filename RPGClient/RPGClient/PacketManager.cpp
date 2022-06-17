#include "stdafx.h"
#include "PacketManager.h"

#include "Game.h"

s3d::int32 PacketManager::sWritePos = 0;
s3d::int32 PacketManager::sReadPos = 0;
s3d::TCPClient PacketManager::sTCPContext = {};
s3d::String PacketManager::sIP = {};
std::unordered_map<char, std::function<void(char*)>> PacketManager::sPacketFuncDict;
char PacketManager::sRecvBuffer[RECV_BUFFER_SIZE] = {};
char PacketManager::sPacketBuffer[PACKET_BUFFER_SIZE] = {};


void PacketManager::Init()
{
	const INI ini{ U"Settings.ini" };

	MK_ERROR(ini, U"Could not open ini file!");

	sIP = ini[U"data.server"];
}

void PacketManager::Shutdown()
{
	if (sTCPContext.isConnected())
	{
		sTCPContext.disconnect();
	}
}

void PacketManager::Recv()
{
	int32 numBytes = static_cast<int32>(sTCPContext.available());

	if (numBytes > 0)
	{
		numBytes = Clamp(numBytes, 0, RECV_BUFFER_SIZE);
		sTCPContext.read(sRecvBuffer, numBytes);

		if (sWritePos + numBytes >= PACKET_BUFFER_SIZE)
		{
			auto remain = sWritePos - sReadPos;

			if (remain > 0)
			{
				CopyMemory(&sPacketBuffer[0], &sPacketBuffer[sReadPos], remain);
			}

			sWritePos = remain;
			sReadPos = 0;
		}

		CopyMemory(&sPacketBuffer[sWritePos], sRecvBuffer, numBytes);
		sWritePos += numBytes;

		auto remain = sWritePos - sReadPos;
		while (remain > 0)
		{
			auto packetSize = sPacketBuffer[sReadPos];

			if (remain >= packetSize)
			{
				if (auto iter = sPacketFuncDict.find(sPacketBuffer[sReadPos + 1]); iter != sPacketFuncDict.end())
				{
					(iter->second)(&sPacketBuffer[sReadPos]);
				}

				remain -= packetSize;
				sReadPos += packetSize;
			}
			else
			{
				break;
			}
		}
	}
}

bool PacketManager::Connect()
{
	using namespace std::chrono;

	const IPv4Address ip{ sIP };
	sTCPContext.connect(ip, PORT_NUM);
	if (not sTCPContext.isConnected())
	{
		std::this_thread::sleep_for(2s);
	}

	if (not sTCPContext.isConnected())
	{
		return false;
	}

	return true;
}

void PacketManager::RegisterPacketFunc(char packetType, std::function<void(char*)> func)
{
	if (auto iter = sPacketFuncDict.find(packetType); iter == sPacketFuncDict.end())
	{
		sPacketFuncDict.emplace(packetType, func);
	}
	else
	{
		MK_ERROR(false, U"PacketType is already registered!");
	}
}

void PacketManager::RemovePacketFunc(char packetType)
{
	if (auto iter = sPacketFuncDict.find(packetType); iter != sPacketFuncDict.end())
	{
		sPacketFuncDict.erase(iter);
	}
	else
	{
		MK_ERROR(false, U"PacketType does not exist");
	}
}

void PacketManager::SendPacket(void* packet, const int32 packetSize)
{
	sTCPContext.send(packet, packetSize);
}

