#pragma once

namespace mk
{
	class Actor
	{
	public:
		void Disconnect();

	public:
		int32_t GetID() const { return mID; }
		void SetID(const int32_t value) { mID = value; }
		std::string GetName() const { return mName; }
		void SetName(std::string_view value) { mName = value; }

		int16_t GetX() const { return mPosX; }
		int16_t GetY() const { return mPosY; }
		std::pair<int16_t, int16_t> GetPos() const { return { mPosX, mPosY }; }
		void SetX(const int16_t value) { mPosX = value; }
		void SetY(const int16_t value) { mPosY = value; }
		void SetPos(const std::pair<int16_t, int16_t>& value) { mPosX = value.first; mPosY = value.second; }

	private:
		int32_t mID = INVALID_VALUE;
		std::string mName = {};
		int16_t mPosX = 0;
		int16_t mPosY = 0;
	};
}