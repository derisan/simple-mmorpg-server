#include "stdafx.h"

#include "IocpBase.h"

std::unique_ptr<mk::IocpBase> gServer = nullptr;

int main()
{
	mk::Log::Init();

	gServer = std::make_unique<mk::IocpBase>();

	bool ret = gServer->Init();

	if (NOT ret)
	{
		return 0;
	}

	gServer->Run();

	gServer->Shutdown();
}