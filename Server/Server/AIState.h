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
	
	class IdleState
		: public AIState
	{
	public:
		IdleState(NPC* owner, bool bAggro);

		virtual void Tick() override;
		
	private:
		bool mbAggro = false;
	};

	class ChaseState
		: public AIState
	{
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

	public:
		ChaseState(NPC* owner);

		virtual void Enter() override;
		virtual void Exit() override;
		virtual void Tick() override;

	private:
		bool isOutOfArea(const vec2& targetPos);
		void setVisit(const short row, const short col);
		bool isVisited(const short row, const short col);
		bool isArrived(const short x, const short y);

	private:
		constexpr static int ENEMY_ATTACK_RANGE = 5;

		vec2 mBorder = {};
		std::priority_queue<Node> mQueue;
		bool mVisited[20][20] = { false, };
		Actor* mTarget = nullptr;
	};

	class RoamingState
		: public AIState
	{
	public:
		RoamingState(NPC* owner);

		virtual void Enter() override;
		virtual void Exit() override;
		virtual void Tick() override;

	private:
		vec2 mBorder = {};
	};
}

