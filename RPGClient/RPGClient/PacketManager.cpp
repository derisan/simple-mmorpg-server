#include "stdafx.h"
#include "PacketManager.h"

#include "Game.h"

s3d::int32 PacketManager::mWritePos = 0;
s3d::int32 PacketManager::mReadPos = 0;
s3d::TCPClient PacketManager::mTCPContext = {};
std::unordered_map<char, std::function<void(char*)>> PacketManager::mPacketFuncDict;
char PacketManager::mRecvBuffer[RECV_BUFFER_SIZE] = {};
char PacketManager::mPacketBuffer[PACKET_BUFFER_SIZE] = {};

void PacketManager::Shutdown()
{
	if (mTCPContext.isConnected())
	{
		mTCPContext.disconnect();
	}
}

void PacketManager::Recv()
{
	int32 numBytes = static_cast<int32>(mTCPContext.available());

	if (numBytes > 0)
	{
		mTCPContext.read(mRecvBuffer, numBytes);

		if (mWritePos + numBytes >= PACKET_BUFFER_SIZE)
		{
			auto remain = mWritePos - mReadPos;

			if (remain > 0)
			{
				CopyMemory(&mPacketBuffer[0], &mPacketBuffer[mReadPos], remain);
			}

			mWritePos = remain;
			mReadPos = 0;
		}

		CopyMemory(&mPacketBuffer[mWritePos], mRecvBuffer, numBytes);
		mWritePos += numBytes;

		auto remain = mWritePos - mReadPos;
		while (remain > 0)
		{
			auto packetSize = mPacketBuffer[mReadPos];

			if (remain >= packetSize)
			{
				if (auto iter = mPacketFuncDict.find(mPacketBuffer[mReadPos + 1]); iter != mPacketFuncDict.end())
				{
					(iter->second)(&mPacketBuffer[mReadPos]);
				}

				remain -= packetSize;
				mReadPos += packetSize;
			}
			else
			{
				break;
			}
		}
	}
}

bool PacketManager::Connect(const IPv4Address& ip, uint16 port)
{
	using namespace std::chrono;

	mTCPContext.connect(ip, port);
	if (not mTCPContext.isConnected())
	{
		std::this_thread::sleep_for(2s);
	}

	if (not mTCPContext.isConnected())
	{
		return false;
	}

	return true;
}

void PacketManager::RegisterPacketFunc(char packetType, std::function<void(char*)> func)
{
	if (auto iter = mPacketFuncDict.find(packetType); iter == mPacketFuncDict.end())
	{
		mPacketFuncDict.emplace(packetType, func);
	}
	else
	{
		Print << U"Packet type: " << packetType << U" is already registered!";
	}
}

void PacketManager::RemovePacketFunc(char packetType)
{
	if (auto iter = mPacketFuncDict.find(packetType); iter != mPacketFuncDict.end())
	{
		mPacketFuncDict.erase(iter);
	}
	else
	{
		Print << U"Packet type: " << packetType << U" not exists!";
	}
}

void PacketManager::SendPacket(void* packet, const int32 packetSize)
{
	mTCPContext.send(packet, packetSize);
}

