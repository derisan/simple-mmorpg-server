#pragma once

#include <queue>

namespace mk
{
	class Actor;
	class NPC;

	class AIState
	{
	public:
		AIState(NPC* owner);
		virtual ~AIState() = default;

		virtual void Enter() {}
		virtual void Exit() {}
		virtual void Tick() {}

	protected:
		NPC* Owner = nullptr;
	};

	struct Node
	{
		short Row = 0;
		short Col = 0;
		int F = 0;
		int Depth = 0;
		short ParentRow = 0;
		short ParentCol = 0;

		bool operator < (const Node& other) const
		{
			return F > other.F;
		}

		bool operator < (const Node& other)
		{
			return F > other.F;
		}
	};

	class ChaseState
		: public AIState
	{
	public:
		ChaseState(NPC* owner);

		virtual void Enter() override;
		virtual void Exit() override;
		virtual void Tick() override;

	private:
		bool isOutOfArea(const vec2& targetPos);
		void setVisit(const short row, const short col);
		bool isVisited(const short row, const short col);

	private:
		vec2 mBorder = {};
		std::priority_queue<Node> mQueue;
		bool mVisited[20][20] = {false, };
		Actor* mTarget = nullptr;
	};
}

