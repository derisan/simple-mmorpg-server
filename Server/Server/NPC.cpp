#include "stdafx.h"
#include "NPC.h"

#include "Protocol.h"

namespace mk
{
	void NPC::AddToViewList(const int id)
	{
		WriteLockGuard guard = { ViewLock };
		ViewList.insert(id);
	}

	void NPC::RemoveFromViewList(const int id)
	{
		WriteLockGuard guard = { ViewLock };
		ViewList.erase(id);
	}
}