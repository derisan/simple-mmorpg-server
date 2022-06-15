#include "stdafx.h"
#include "MainScene.h"

#include <chrono>

#include "ActorManager.h"
#include "TileMap.h"
#include "PacketManager.h"

void MainScene::Enter()
{
	TileMap::WaitMapLoading();

	PacketManager::RegisterPacketFunc(SC_MOVE_PLAYER, [](char* p) {
		SC_MOVE_PLAYER_PACKET* packet = reinterpret_cast<SC_MOVE_PLAYER_PACKET*>(p);
		ActorManager::MoveActor(packet->id,
			packet->x,
			packet->y);
	});
}

void MainScene::Exit()
{
	PacketManager::RemovePacketFunc(SC_MOVE_PLAYER);
}

void MainScene::ProcessInput()
{
	BaseScene::ProcessInput();
}

void MainScene::Update(const float deltaTime)
{
	UNREFERENCED_PARAMETER(deltaTime);

	pollKeyDown();
}

void MainScene::Render()
{
	ClearPrint();
	Point myPos = mActor->GetPos();
	Print << myPos;

	TileMap::RenderMap(myPos);
}

void MainScene::pollKeyDown()
{
	using namespace std::chrono;

	char direction = -1;

	if (KeyUp.down())
	{
		direction = 0;
	}
	else if (KeyDown.down())
	{
		direction = 1;
	}
	else if (KeyLeft.down())
	{
		direction = 2;
	}
	else if (KeyRight.down())
	{
		direction = 3;
	}

	if (direction >= 0)
	{
		CS_MOVE_PACKET packet = {};
		packet.size = sizeof(packet);
		packet.type = CS_MOVE;
		packet.direction = direction;
		packet.client_time = static_cast<unsigned>(duration_cast<milliseconds>(
			system_clock::now().time_since_epoch()).count());
		PacketManager::SendPacket(&packet, packet.size);
	}
}
