#pragma once

namespace mk
{
	class Actor
	{
	public:
		void Disconnect();

	public:
		int GetID() const { return mID; }
		void SetID(const int value) { mID = value; }
		std::string GetName() const { return mName; }
		void SetName(std::string_view value) { mName = value; }

		short GetX() const { return mPosX; }
		short GetY() const { return mPosY; }
		std::pair<short, short> GetPos() const { return { mPosX, mPosY }; }
		void SetX(const short value) { mPosX = value; }
		void SetY(const short value) { mPosY = value; }
		void SetPos(const std::pair<short, short>& value) { mPosX = value.first; mPosY = value.second; }
		void SetPos(const short x, const short y) { mPosX = x; mPosY = y; }

	private:
		int mID = INVALID_VALUE;
		std::string mName = {};
		short mPosX = 0;
		short mPosY = 0;
	};
}