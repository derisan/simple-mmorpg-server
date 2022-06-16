#include "stdafx.h"
#include "BaseScene.h"

#include "PacketManager.h"

void BaseScene::ProcessInput()
{
	PacketManager::Recv();
}
