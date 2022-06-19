#include "stdafx.h"
#include "AIState.h"

#include "NPC.h"
#include "TileMap.h"
#include "IocpBase.h"
#include "SectorManager.h"
#include "Session.h"

namespace mk
{
	int GetHeuristics(const vec2& a, const vec2& b)
	{
		return abs(a.x - b.x) + abs(a.y - b.y);
	}

	AIState::AIState(NPC* owner)
		: Owner{ owner }
	{

	}

	ChaseState::ChaseState(NPC* owner)
		: AIState{ owner }
	{

	}

	void ChaseState::Enter()
	{
		auto ownerPos = Owner->GetPos();
		mBorder.x = (ownerPos.x / 20) * 20;
		mBorder.y = (ownerPos.y / 20) * 20;

		auto targetID = Owner->GetTargetID();
		mTarget = gClients[targetID];
	}

	void ChaseState::Exit()
	{
		mTarget = nullptr;
	}

	void ChaseState::Tick()
	{
		static short dx[] = { 1, -1, 0, 0 };
		static short dy[] = { 0, 0, 1, -1 };

		auto targetPos = mTarget->GetPos();

		bool bInRange = isInRange(targetPos);
		if (NOT bInRange)
		{
			setInactive();
			return;
		}

		bool isOut = isOutOfArea(targetPos);
		if (isOut)
		{
			setInactive();
			return;
		}

		mQueue = std::priority_queue<Node>();
		ZeroMemory(mVisited, sizeof(mVisited));

		Node start = { targetPos.y, targetPos.x, 0, 0, -1, -1 };
		setVisit(targetPos.y, targetPos.x);
		mQueue.push(start);

		auto goalPos = Owner->GetPos();

		while (NOT mQueue.empty())
		{
			Node node = mQueue.top();
			mQueue.pop();

			if (node.Col == goalPos.x &&
				node.Row == goalPos.y)
			{
				auto newX = node.ParentCol;
				auto newY = node.ParentRow;

				bool bArrived = isArrived(newX, newY);
				if (bArrived)
				{
					auto ownerID = Owner->GetID();
					static_cast<Session*>(mTarget)->OnHit(ownerID);
				}
				else
				{
					Owner->SetPos(newX, newY);
					SectorManager::SendNpcMoveToViewList(Owner);
				}

				// 길을 찾은 경우 함수 리턴
				return;
			}

			for (auto i = 0; i < 4; ++i)
			{
				auto newX = node.Col + dx[i];
				auto newY = node.Row + dy[i];
				
				if (NOT isVisited(newY, newX)
					&& NOT TileMap::IsSolid(newY, newX)
					&& NOT isOutOfArea(vec2(newX, newY)))
				{
					setVisit(newY, newX);
					Node newNode = {};
					newNode.Row = newY;
					newNode.Col = newX;
					newNode.Depth = node.Depth + 1;
					newNode.F = GetHeuristics(goalPos, vec2(newX, newY))
						+ newNode.Depth;
					newNode.ParentRow = node.Row;
					newNode.ParentCol = node.Col;
					mQueue.push(newNode);
				}
			}
		}

		// 길을 못 찾았을 경우 실행되는 코드
		setInactive();
	}

	bool ChaseState::isOutOfArea(const vec2& targetPos)
	{
		if (targetPos.x < mBorder.x ||
			targetPos.x > mBorder.x + 19)
		{
			return true;
		}

		if (targetPos.y < mBorder.y ||
			targetPos.y > mBorder.y + 19)
		{
			return true;
		}

		return false;
	}

	void ChaseState::setVisit(const short row, const short col)
	{
		mVisited[row % 20][col % 20] = true;
	}

	bool ChaseState::isVisited(const short row, const short col)
	{
		return mVisited[row % 20][col % 20];
	}

	bool ChaseState::isInRange(const vec2& targetPos)
	{
		auto ownerPos = Owner->GetPos();

		if (targetPos.x < ownerPos.x - ENEMY_ATTACK_RANGE ||
			targetPos.x > ownerPos.x + ENEMY_ATTACK_RANGE ||
			targetPos.y < ownerPos.y - ENEMY_ATTACK_RANGE ||
			targetPos.y > ownerPos.y + ENEMY_ATTACK_RANGE)
		{
			return false;
		}

		return true;
	}

	bool ChaseState::isArrived(const short x, const short y)
	{
		return x == -1 && y == -1;
	}

	void ChaseState::setInactive()
	{
		WriteLockGuard guard = { Owner->ActorLock };
		Owner->SetActive(false);
		Owner->SetTargetID(INVALID_VALUE);
	}

}