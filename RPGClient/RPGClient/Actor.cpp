#include "stdafx.h"
#include "Actor.h"

#include "ResourceManager.h"

void Actor::Render()
{
	int32 x = mX % 20 * 32;
	int32 y = mY % 20 * 32;

	Rect{ x, y, 32, 32 }(TextureAsset(mTexName)).draw();
	PutText(mName, Vec2{ x + 16, y - 16 });
}

void Actor::SetRace(const int16 value)
{
	mRace = value;
	mTexName = ResourceManager::GetTexName(mRace);
}
