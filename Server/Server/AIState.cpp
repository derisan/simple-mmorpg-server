#include "stdafx.h"
#include "AIState.h"

#include "NPC.h"
#include "TileMap.h"
#include "IocpBase.h"
#include "SectorManager.h"
#include "Session.h"
#include "Random.h"

namespace mk
{
	int GetHeuristics(const vec2& a, const vec2& b)
	{
		return abs(a.x - b.x) + abs(a.y - b.y);
	}

	bool IsInRange(const vec2& a, const vec2& b, const int distance)
	{
		if (a.x < b.x - distance 
			|| a.x > b.x + distance 
			|| a.y < b.y - distance 
			|| a.y > b.y + distance)
		{
			return false;
		}

		return true;
	}

	AIState::AIState(NPC* owner)
		: Owner{ owner }
	{

	}

	IdleState::IdleState(NPC* owner, bool bAggro)
		: AIState{ owner }
		, mbAggro{ bAggro }
	{

	}

	void IdleState::Tick()
	{
		if (NOT mbAggro)
		{
			return;
		}

		std::unordered_set<id_t> viewList;
		{
			ReadLockGuard guard = { Owner->ViewLock };
			viewList = Owner->ViewList;
		}

		const auto& ownerPos = Owner->GetPos();

		for (auto actorID : viewList)
		{
			auto otherPos = gClients[actorID]->GetPos();
			bool bInRange = IsInRange(ownerPos, otherPos, 5);

			if (bInRange)
			{
				Owner->SetTargetID(actorID);
				Owner->ChangeState(new ChaseState{ Owner });
				break;
			}
		}
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

		const auto& targetPos = mTarget->GetPos();
		const auto& ownerPos = Owner->GetPos();

		bool bInRange = IsInRange(ownerPos, targetPos, ENEMY_ATTACK_RANGE);
		if (NOT bInRange)
		{
			Owner->BackToPreviousState();
			return;
		}

		bool isOut = isOutOfArea(targetPos);
		if (isOut)
		{
			Owner->BackToPreviousState();
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
		Owner->BackToPreviousState();
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

	bool ChaseState::isArrived(const short x, const short y)
	{
		return x == -1 && y == -1;
	}

	RoamingState::RoamingState(NPC* owner)
		: AIState{ owner }
	{

	}

	void RoamingState::Enter()
	{
		auto ownerPos = Owner->GetPos();
		mBorder.x = (ownerPos.x / 20) * 20;
		mBorder.y = (ownerPos.y / 20) * 20;
	}

	void RoamingState::Exit()
	{

	}

	void RoamingState::Tick()
	{
		Owner->ViewLock.ReadLock();
		if (Owner->ViewList.empty())
		{
			Owner->ViewLock.ReadUnlock();

			WriteLockGuard guard = { Owner->ActorLock };
			Owner->SetActive(false);
		}
		else
		{
			Owner->ViewLock.ReadUnlock();
		}

		char direction = Random::RandInt(0, 3);

		auto ownerPos = Owner->GetPos();

		switch (direction)
		{
		case 0:
			if (ownerPos.y > mBorder.y
				&& NOT TileMap::IsSolid(ownerPos.y - 1, ownerPos.x))
				ownerPos.y--;
			break;
		case 1:
			if (ownerPos.y < mBorder.y + 19
				&& NOT TileMap::IsSolid(ownerPos.y + 1, ownerPos.x))
				ownerPos.y++;
			break;
		case 2:
			if (ownerPos.x > mBorder.x
				&& NOT TileMap::IsSolid(ownerPos.y, ownerPos.x - 1))
				ownerPos.x--;
			break;
		case 3:
			if (ownerPos.x < mBorder.x + 19
				&& NOT TileMap::IsSolid(ownerPos.y, ownerPos.x + 1))
				ownerPos.x++;
			break;
		}

		Owner->SetPos(ownerPos);
		SectorManager::SendNpcMoveToViewList(Owner);
	}



}