#include <Siv3D.hpp> // OpenSiv3D v0.6.4

#include "Game.h"

std::unique_ptr<Game> gGame = nullptr;

void Main()
{
	System::SetTerminationTriggers(UserAction::CloseButtonClicked);

	gGame = std::make_unique<Game>();
	gGame->Init();

	while (System::Update())
	{
		gGame->Run();
	}

	gGame->Shutdown();
}
