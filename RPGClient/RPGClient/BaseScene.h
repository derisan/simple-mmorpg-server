#pragma once

class BaseScene
{
public:
	virtual ~BaseScene() = default;

	virtual void Enter() {}
	virtual void Exit() {}
	virtual void ProcessInput();
	virtual void Update(const float deltaTime) { UNREFERENCED_PARAMETER(deltaTime); }
	virtual void Render() {}
};

