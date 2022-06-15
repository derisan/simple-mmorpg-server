#pragma once

#include "BaseScene.h"

class Actor;

class MainScene :
    public BaseScene
{
public:
	virtual void Enter() override;
	virtual void Exit() override;
	virtual void ProcessInput() override;
	virtual void Update(const float deltaTime) override;
	virtual void Render() override;

public:
	void SetActor(Actor* actor) { mActor = actor; }

private:
	Actor* mActor = nullptr;
};

