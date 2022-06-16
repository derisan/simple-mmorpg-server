#pragma once

class Actor
{
public:
	void Render();

public:
	int16 GetX() const { return mX; }
	int16 GetY() const { return mY; }
	int16 GetRace() const { return mRace; }
	Point GetPos() const { return Point{ mX, mY }; }
	int32 GetID() const { return mID; }
	String GetName() const { return mName; }

	void SetX(const int16 value) { mX = value; }
	void SetY(const int16 value) { mY = value; }
	void SetRace(const int16 value);
	void SetID(const int32 value) { mID = value; }
	void SetName(const String& value) { mName = value; }

private:
	int16 mX = 0;
	int16 mY = 0;
	int16 mRace = 0;
	int32 mID = 0;
	String mName = U"Default";
	String mTexName = {};
};
