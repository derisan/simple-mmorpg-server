#pragma once

#include "BaseScene.h"

class MainScene :
    public BaseScene
{
public:
	virtual void Enter() override;
	virtual void Exit() override;
	virtual void ProcessInput() override;
	virtual void Update(const float deltaTime) override;
	virtual void Render() override;

private:
};

