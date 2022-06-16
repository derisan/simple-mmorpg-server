#include "stdafx.h"
#include "Game.h"

#include "LoginScene.h"
#include "TileMap.h"
#include "ActorManager.h"
#include "PacketManager.h"
#include "ResourceManager.h"

std::unique_ptr<Game> gGame;

Game::Game()
{

}

Game::~Game()
{

}

void Game::Init()
{
	TileMap::Init();
	TileMap::LoadMapAsync(ASSET_PATH(WorldMap.txt));
	ResourceManager::Init();
	PacketManager::Init();
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
	MK_ERROR(newScene, U"newScene is nullptr");

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
	ActorManager::RenderActors();
}
