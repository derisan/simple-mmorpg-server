#include "stdafx.h"
#include "Actor.h"

#include "ResourceManager.h"

void Actor::Render()
{
	int32 x = mX % 20 * 32;
	int32 y = mY % 20 * 32;

	TextureAsset{ mTexName }.draw(x, y);
	PutText(U"Lv." + Format(mLevel), Vec2{ x + 16, y - 32 });
	PutText(mName, Vec2{x + 16, y - 16});
	PutText(Format(mCurrentHP) + U" | " + Format(mMaxHP), Vec2{ x + 16, y + 42 });
}

void Actor::SetRace(const int16 value)
{
	mRace = value;
	mTexName = ResourceManager::GetTexName(mRace);
}
