#pragma once

#include "BaseScene.h"

class LoginScene :
    public BaseScene
{
public:
	virtual void Enter() override;
	virtual void Exit() override;
	virtual void ProcessInput() override;
	virtual void Render() override;

private:
	void renderBackground();
	void renderUI();
	void doLogin(const String& loginID);

private:
	TextEditState mUserInput = {};
	Texture mBackground = {};
	Texture mTitle = {};
};

