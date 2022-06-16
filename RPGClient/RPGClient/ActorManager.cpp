#include "stdafx.h"
#include "ActorManager.h"

#include "ResourceManager.h"

std::unordered_map<s3d::int32, Actor> ActorManager::sActors;

Actor& ActorManager::RegisterActor(const int32 id, const int16 x, const int16 y,
	const String& name)
{
	Actor actor = {};
	actor.SetID(id);
	actor.SetX(x);
	actor.SetY(y);
	actor.SetRace(0);
	actor.SetName(name);

	if (auto iter = sActors.find(id); iter == sActors.end())
	{
		sActors.emplace(id, std::move(actor));
	}
	else
	{
		Console << U"Actor id: " << id << U" already exists!(RegisterActor())";
		sActors.erase(id);
		sActors.emplace(id, std::move(actor));
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
		Console << U"Actor id : " << id << U" does not exist!(RemoveActor())";
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
		Console << U"Actor id : " << id << U" does not exist!(MoveActor())";
	}
}

void ActorManager::RenderActors()
{
	for (auto& [id, actor] : sActors)
	{
		actor.Render();
	}
}
