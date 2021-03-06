#include <Siv3D.hpp> // OpenSiv3D v0.6.4

#include "Game.h"

void Main()
{
	System::SetTerminationTriggers(UserAction::CloseButtonClicked);
	Window::Resize(640, 640);

	gGame = std::make_unique<Game>();
	gGame->Init();

	while (System::Update())
	{
		gGame->Run();
	}

	gGame->Shutdown();
}
