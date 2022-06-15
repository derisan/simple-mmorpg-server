#include "stdafx.h"
#include "Game.h"

#include "LoginScene.h"
#include "PacketManager.h"
#include "TileMap.h"

std::unique_ptr<Game> gGame;

Game::Game()
{

}

Game::~Game()
{

}

void Game::Init()
{
	TileMap::LoadMapAsync(ASSET_PATH(WorldMap.txt));

	mActiveScene = std::make_unique<LoginScene>();
	mActiveScene->Enter();
}

void Game::Shutdown()
{
	if (mActiveScene)
	{
		mActiveScene->Exit();
		mActiveScene = nullptr;
	}

	PacketManager::Shutdown();
}

void Game::Run()
{
	processInput();
	update();
	render();
}

void Game::ChangeScene(BaseScene* newScene)
{
	MK_ASSERT(newScene);

	mActiveScene->Exit();
	mActiveScene.reset(newScene);
	mActiveScene->Enter();
}

void Game::processInput()
{
	PacketManager::Recv();
	mActiveScene->ProcessInput();
}

void Game::update()
{
	const float deltaTime = static_cast<float>(Scene::DeltaTime());
	mActiveScene->Update(deltaTime);
}

void Game::render()
{
	mActiveScene->Render();
}
