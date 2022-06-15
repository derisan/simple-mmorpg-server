#include "stdafx.h"
#include "LoginScene.h"

#include "ActorManager.h"
#include "Game.h"
#include "MainScene.h"
#include "PacketManager.h"

void LoginScene::Enter()
{
	mBackground = Texture{ ASSET_PATH(Login_Background.png) };
	mTitle = Texture{ ASSET_PATH(Login_Title.png) };

	PacketManager::RegisterPacketFunc(SC_LOGIN_INFO, [](char* p) {
		SC_LOGIN_INFO_PACKET* packet = reinterpret_cast<SC_LOGIN_INFO_PACKET*>(p);
		// TODO : 로그인 성공 여부 따져서 메인 씬으로 전환
		auto& actor = ActorManager::RegisterActor(packet->id, packet->x, packet->y);

		auto mainScene = new MainScene;
		mainScene->SetActor(&actor);

		gGame->ChangeScene(mainScene);
		});
}

void LoginScene::Exit()
{
	PacketManager::RemovePacketFunc(SC_LOGIN_INFO);
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
	constexpr static int32 maxChars = 19;

	const auto ScreenSize = Scene::Size();
	float xPos = (ScreenSize.x - textBoxWidth) / 2.0f;

	SimpleGUI::Headline(
		U"계정",
		Vec2{ xPos + 2, ScreenSize.y - 163.0f });

	SimpleGUI::TextBox(
		mUserInput,
		Vec2{ xPos, ScreenSize.y - 125.0f },
		textBoxWidth,
		maxChars);

	xPos = (ScreenSize.x - buttonWidth) / 2.0f;

	bool isClicked = SimpleGUI::Button(U"로그인", Vec2{ xPos, ScreenSize.y - 75.0f });
	if (isClicked)
	{
		doLogin(mUserInput.text);
	}
}

void LoginScene::doLogin(const String& loginID)
{
	// TODO : ip, port 매직 넘버 수정
	const IPv4Address ip = IPv4Address::Localhost();
	bool bConnect = PacketManager::Connect(ip, 4000);

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
