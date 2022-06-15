#include "stdafx.h"
#include "Actor.h"

void Actor::Render()
{
	Rect{ mX % 20 * 32, mY % 20 * 32, 32, 32 }(mTexture).draw();
}
