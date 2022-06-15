#pragma once

class Actor
{
public:
	void Render(const Font& font);

public:
	int16 GetX() const { return mX; }
	int16 GetY() const { return mY; }
	Point GetPos() const { return Point{ mX, mY }; }
	int32 GetID() const { return mID; }
	String GetName() const { return mName; }

	void SetX(const int16 value) { mX = value; }
	void SetY(const int16 value) { mY = value; }
	void SetID(const int32 value) { mID = value; }
	void SetName(const String& value) { mName = value; }
	void SetTexture(const Texture* tex) { mTexture = tex; }

private:
	int16 mX = 0;
	int16 mY = 0;
	int32 mID = 0;
	String mName = U"Default";

	const Texture* mTexture = nullptr;
};
