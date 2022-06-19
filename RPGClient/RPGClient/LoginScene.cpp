#include "stdafx.h"
#include "LoginScene.h"

#include "ActorManager.h"
#include "ChatManager.h"
#include "Game.h"
#include "MainScene.h"
#include "PacketManager.h"

void LoginScene::Enter()
{
	mBackground = Texture{ ASSET_PATH(Login_Background.png) };
	mTitle = Texture{ ASSET_PATH(Login_Title.png) };

	PacketManager::RegisterPacketFunc(SC_LOGIN_OK, [this](char* p) {
		SC_LOGIN_OK_PACKET* packet = reinterpret_cast<SC_LOGIN_OK_PACKET*>(p);
		auto& actor = ActorManager::RegisterActor(packet->id,
			packet->race,
			packet->x,
			packet->y,
			packet->level,
			packet->hp,
			packet->hpmax,
			mUserInput.text,
			packet->exp);

		auto mainScene = new MainScene;
		mainScene->SetActor(&actor);

		gGame->ChangeScene(mainScene);
		});

	PacketManager::RegisterPacketFunc(SC_LOGIN_FAIL, [](char* p) {
		SC_LOGIN_FAIL_PACKET* packet = reinterpret_cast<SC_LOGIN_FAIL_PACKET*>(p);
		switch (packet->reason)
		{
		case 0: ChatManager::AddChat(-1, "Login Failed: Invalid name!"); break;
		case 1: ChatManager::AddChat(-1, "Login Failed: Name already playing!"); break;
		case 2: ChatManager::AddChat(-1, "Login Failed: Server is full!"); break;
		}
		PacketManager::Disconnect();
	});
}

void LoginScene::Exit()
{
	PacketManager::RemovePacketFunc(SC_LOGIN_OK);
}

void LoginScene::ProcessInput()
{
	BaseScene::ProcessInput();
}

void LoginScene::Render()
{
	renderBackground();
	renderUI();
}

void LoginScene::renderBackground()
{
	const auto ScreenSize = Scene::Size();
	Rect{ 0, 0, ScreenSize.x, ScreenSize.y }(mBackground).draw();
	Rect{ 20, 150, ScreenSize.x - 40, 67 }(mTitle).draw();
}

void LoginScene::renderUI()
{
	constexpr static float textBoxWidth = 300.0f;
	constexpr static float buttonWidth = 100.0f;

	const auto ScreenSize = Scene::Size();
	float xPos = (ScreenSize.x - textBoxWidth) / 2.0f;

	SimpleGUI::Headline(
		U"계정",
		Vec2{ xPos + 2, ScreenSize.y - 163.0f });

	SimpleGUI::TextBox(
		mUserInput,
		Vec2{ xPos, ScreenSize.y - 125.0f },
		textBoxWidth,
		NAME_SIZE);

	xPos = (ScreenSize.x - buttonWidth) / 2.0f;

	bool isClicked = SimpleGUI::Button(U"로그인", Vec2{ xPos, ScreenSize.y - 75.0f });
	if (isClicked)
	{
		doLogin(mUserInput.text);
	}
}

void LoginScene::doLogin(const String& loginID)
{
	bool bConnect = PacketManager::Connect();
	if (not bConnect)
	{
		System::Exit();
	}
	
	std::string userID = loginID.narrow();

	CS_LOGIN_PACKET packet = {};
	packet.size = sizeof(packet);
	packet.type = CS_LOGIN;
	CopyMemory(packet.name, userID.data(), userID.size());
	PacketManager::SendPacket(&packet, packet.size);
}
