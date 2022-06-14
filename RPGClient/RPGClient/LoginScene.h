#pragma once

#include "BaseScene.h"

class LoginScene :
    public BaseScene
{
public:
	virtual void Enter() override;
	virtual void Update(const float deltaTime) override;
	virtual void Render() override;

private:
	void renderBackground();
	void renderUI();

private:
	TextEditState mUserInput = {};
	Texture mBackground = {};
	Texture mTitle = {};
};

