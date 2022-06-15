#include "stdafx.h"
#include "MainScene.h"

#include "TileMap.h"

void MainScene::Enter()
{

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

	if (KeyLeft.down())
	{
		if (mTempPos.x > 0) mTempPos.x--;
	}

	if (KeyRight.down())
	{
		if (mTempPos.x < 1999) mTempPos.x++;
	}

	if (KeyUp.down())
	{
		if (mTempPos.y > 0) mTempPos.y--;
	}

	if (KeyDown.down())
	{
		if (mTempPos.y < 1999) mTempPos.y++;
	}

	if (KeyEnter.down())
	{
		mTempPos = Point{ 1999, 1999 };
	}
}

void MainScene::Render()
{
	ClearPrint();
	Print << U"MainScene";
	Print << mTempPos;

	TileMap::RenderMap(mTempPos);

	int32 x = (mTempPos.x % 20) * 32;
	int32 y = (mTempPos.y % 20) * 32;
	Rect{ x, y, 32, 32 }.draw(Palette::Orange);
}
