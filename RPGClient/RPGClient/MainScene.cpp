﻿#include "stdafx.h"
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
}

void MainScene::Render()
{
	ClearPrint();
	Print << U"MainScene";

	TileMap::RenderMap({ 0, 0 });
}
