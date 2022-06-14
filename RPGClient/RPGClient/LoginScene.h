#pragma once

#include "BaseScene.h"

class LoginScene :
    public BaseScene
{
public:
	virtual void Update(const float deltaTime) override;
	virtual void Render() override;
	
};

