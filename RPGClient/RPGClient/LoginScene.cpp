#include "stdafx.h"
#include "LoginScene.h"

constexpr float textBoxWidth = 300.0f;
constexpr float buttonWidth = 100.0f;
constexpr int32 maxChars = 19;

void LoginScene::Enter()
{
	mBackground = Texture{ U"Assets/Login_Background.png" };
	mTitle = Texture{ U"Assets/Login_Title.png" };
}

void LoginScene::Update(const float deltaTime)
{
	UNREFERENCED_PARAMETER(deltaTime);
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
	Rect{ 20, 150, 760, 67}(mTitle).draw();
}

void LoginScene::renderUI()
{
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
		Print << mUserInput.text;
	}
}
