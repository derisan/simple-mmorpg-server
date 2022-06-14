#pragma once

class Game;
class BaseScene;

extern std::unique_ptr<Game> gGame;
extern TCPClient gTCPClient;

class Game
{
public:
	Game();
	~Game();

	void Init();
	void Shutdown();
	void Run();
	void ChangeScene(BaseScene* newScene);

private:
	void processInput();
	void update();
	void render();

private:
	std::unique_ptr<BaseScene> mActiveScene = nullptr;
};


