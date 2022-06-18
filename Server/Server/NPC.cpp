#include "stdafx.h"
#include "NPC.h"

#include "Protocol.h"

namespace mk
{
	void NPC::Shutdown()
	{
		Actor::Shutdown();

		bool mbHostile = false;
		NpcBehaviorType mBehaviorType = NpcBehaviorType::NONE;
		NpcMoveType mMoveType = NpcMoveType::NONE;
	}

	void NPC::Tick()
	{
		if (NOT IsActive())
		{
			return;
		}

	}

	bool NPC::AddToViewList(const id_t id, const bool bSendMove /*= false*/)
	{
		if (id > MAX_USER)
		{
			return false;
		}

		WriteLockGuard guard = { ViewLock };
		auto [_, bInsert] = ViewList.insert(id);
		return bInsert;
	}

	bool NPC::RemoveFromViewList(const id_t id)
	{
		WriteLockGuard guard = { ViewLock };
		auto cnt = ViewList.erase(id);
		return cnt == 1;
	}
}