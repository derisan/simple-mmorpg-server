#pragma once

class Actor
{
public:
	void Render();

public:
	int16 GetX() const { return mX; }
	int16 GetY() const { return mY; }
	int16 GetRace() const { return mRace; }
	int16 GetLevel() const { return mLevel; }
	int32 GetExp() const { return mExp; }
	int32 GetCurrentHP() const { return mCurrentHP; }
	int32 GetMaxHP() const { return mMaxHP; }
	Point GetPos() const { return Point{ mX, mY }; }
	int32 GetID() const { return mID; }
	String GetName() const { return mName; }

	void SetX(const int16 value) { mX = value; }
	void SetY(const int16 value) { mY = value; }
	void SetRace(const int16 value);
	void SetLevel(const int16 value) { mLevel = value; }
	void SetExp(const int32 value) { mExp = value; }
	void SetCurrentHP(const int32 value) { mCurrentHP = value; }
	void SetMaxHP(const int32 value) { mMaxHP = value; }
	void SetID(const int32 value) { mID = value; }
	void SetName(const String& value) { mName = value; }

private:
	int16 mX = 0;
	int16 mY = 0;
	int16 mRace = 0;
	int16 mLevel = 0;
	int32 mExp = 0;
	int32 mCurrentHP = 0;
	int32 mMaxHP = 0;
	int32 mID = 0;
	String mName = U"Default";
	String mTexName = {};
};
