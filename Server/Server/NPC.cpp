#include "stdafx.h"
#include "NPC.h"

#include "AIState.h"
#include "Protocol.h"

namespace mk
{

	NPC::NPC()
		: mCurrentState{ std::make_shared<AIState>(this) }
	{

	}

	void NPC::Shutdown()
	{
		Actor::Shutdown();

		bool mbHostile = false;
		NpcBehaviorType mBehaviorType = NpcBehaviorType::NONE;
		NpcMoveType mMoveType = NpcMoveType::NONE;
		mTargetID = INVALID_VALUE;
		mCurrentState = nullptr;
		mPrevState = nullptr;
	}

	void NPC::Tick()
	{
		if (NOT IsActive())
		{
			return;
		}

		mCurrentState->Tick();
	}

	bool NPC::AddToViewList(const id_t id, const bool bSendMove /*= false*/)
	{
		if (id >= MAX_USER)
		{
			return false;
		}

		SetActive(true);

		WriteLockGuard guard = { ViewLock };
		auto [_, bInsert] = ViewList.insert(id);
		return bInsert;
	}

	bool NPC::RemoveFromViewList(const id_t id)
	{
		WriteLockGuard guard = { ViewLock };
		auto cnt = ViewList.erase(id);
		if (ViewList.empty())
		{
			SetActive(false);
		}
		return cnt == 1;
	}

	void NPC::OnHit(const id_t hitterID)
	{
		SetActive(true);
		mTargetID = hitterID;
		ChangeState(new ChaseState{ this });
	}

	void NPC::ChangeState(AIState* newState)
	{
		mCurrentState->Exit();
		mPrevState = mCurrentState;
		mCurrentState.reset(newState);
		mCurrentState->Enter();
	}

	void NPC::BackToPreviousState()
	{
		mCurrentState->Exit();
		mCurrentState.swap(mPrevState);
		mCurrentState->Enter();
	}

}