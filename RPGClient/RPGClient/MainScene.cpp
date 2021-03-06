#include "stdafx.h"
#include "MainScene.h"

#include <chrono>

#include "ActorManager.h"
#include "ChatManager.h"
#include "TileMap.h"
#include "PacketManager.h"

void MainScene::Enter()
{
	TileMap::WaitMapLoading();

	PacketManager::RegisterPacketFunc(SC_MOVE_OBJECT, [](char* p) {
		SC_MOVE_OBJECT_PACKET* packet = reinterpret_cast<SC_MOVE_OBJECT_PACKET*>(p);
		ActorManager::MoveActor(packet->id,
			packet->x,
			packet->y);
	});

	PacketManager::RegisterPacketFunc(SC_ADD_OBJECT, [](char* p) {
		SC_ADD_OBJECT_PACKET* packet = reinterpret_cast<SC_ADD_OBJECT_PACKET*>(p);

		std::string_view name = packet->name;

		ActorManager::RegisterActor(packet->id,
			packet->race,
			packet->x,
			packet->y,
			packet->level,
			packet->hp,
			packet->hpmax,
			Unicode::Widen(name));
	});

	PacketManager::RegisterPacketFunc(SC_REMOVE_OBJECT, [](char* p) {
		SC_REMOVE_OBJECT_PACKET* packet = reinterpret_cast<SC_REMOVE_OBJECT_PACKET*>(p);
		ActorManager::RemoveActor(packet->id);
	});

	PacketManager::RegisterPacketFunc(SC_CHAT, [](char* p) {
		SC_CHAT_PACKET* packet = reinterpret_cast<SC_CHAT_PACKET*>(p);
		ChatManager::AddChat(packet->id, packet->mess);
	});

	PacketManager::RegisterPacketFunc(SC_STAT_CHANGE, [](char* p) {
		SC_STAT_CHANGE_PACKET* packet = reinterpret_cast<SC_STAT_CHANGE_PACKET*>(p);
		ActorManager::ChangeStat(packet->id,
			packet->level,
			packet->exp,
			packet->hp,
			packet->hpmax);
	});
}

void MainScene::Exit()
{
	
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
	Point myPos = mActor->GetPos();
	TileMap::RenderMap(myPos);

	if (mbChatFlag)
	{
		ChatManager::TakeUserChat();
	}

	PutText(U"EXP: " + Format(mActor->GetExp()), Vec2{ 320, 578 });
	PutText(U"POS: " + Format(myPos), Vec2{ 320, 600 });
	PutText(U"ID: " + Format(mActor->GetID()), Vec2{ 320, 622 });
	//PutText(U"LEVEL: " + Format(mActor->GetLevel()), Vec2{ 320, 10 });
	//PutText(U"HP: " + Format(mActor->GetCurrentHP()) + U" | " + Format(mActor->GetMaxHP()),
	//	Vec2{ 320, 30 });


	ActorManager::RenderActors(myPos);
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

	if (KeyEnter.down())
	{
		mbChatFlag = !mbChatFlag;

		if (not mbChatFlag)
		{
			ChatManager::SendCurrentChat();
		}
	}

	if (KeySpace.down())
	{
		CS_ATTACK_PACKET packet = {};
		packet.size = sizeof(packet);
		packet.type = CS_ATTACK;
		PacketManager::SendPacket(&packet, packet.size);
	}
}
