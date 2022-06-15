#include "stdafx.h"
#include "MainScene.h"

#include "Actor.h"
#include "TileMap.h"

void MainScene::Enter()
{
	TileMap::WaitMapLoading();
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
}

void MainScene::Render()
{
	ClearPrint();
	Point myPos = mActor->GetPos();
	Print << myPos;

	TileMap::RenderMap(myPos);
}
