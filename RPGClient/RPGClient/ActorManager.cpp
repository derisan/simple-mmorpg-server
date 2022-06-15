#include "stdafx.h"
#include "ActorManager.h"

#include "ResourceManager.h"

std::unordered_map<s3d::int32, Actor> ActorManager::sActors;

Actor& ActorManager::RegisterActor(const int32 id, const int16 x, const int16 y)
{
	Actor actor = {};
	actor.SetID(id);
	actor.SetX(x);
	actor.SetY(y);

	// TODO : 종족에 따라 텍스쳐 변경
	auto actorTexture = ResourceManager::GetTexture(0);
	actor.SetTexture(actorTexture);

	if (auto iter = sActors.find(id); iter == sActors.end())
	{
		sActors.emplace(id, actor);
	}
	else
	{
		Print << U"Actor id: " << id << U" already exists!";
		MK_ASSERT(false);
	}

	return sActors[id];
}

void ActorManager::RemoveActor(const int32 id)
{
	if (auto iter = sActors.find(id); iter != sActors.end())
	{
		sActors.erase(iter);
	}
	else
	{
		Print << U"Actor id : " << id << U" does not exist!(RemoveActor())";
		MK_ASSERT(false);
	}
}

void ActorManager::MoveActor(const int32 id, const int16 x, const int16 y)
{
	if (auto iter = sActors.find(id); iter != sActors.end())
	{
		(iter->second).SetX(x);
		(iter->second).SetY(y);
	}
	else
	{
		Print << U"Actor id : " << id << U" does not exist!(MoveActor())";
		MK_ASSERT(false);
	}
}

void ActorManager::RenderActors()
{
	for (auto& [id, actor] : sActors)
	{
		actor.Render();
	}
}
