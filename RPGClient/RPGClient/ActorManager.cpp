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

void ActorManager::RenderActors(const Point& myPos)
{
	for (auto& [id, actor] : sActors)
	{
		bool bSameArea = isInSameArea(myPos, actor.GetPos());

		if (bSameArea)
		{
			actor.Render();
		}
	}
}

s3d::String ActorManager::GetActorName(const int32 id)
{
	if (auto iter = sActors.find(id); iter != sActors.end())
	{
		return (iter->second).GetName();
	}
	else
	{
		return U"Default";
	}
}

bool ActorManager::isInSameArea(const Point& myPos, const Point& otherPos)
{
	Point leftTop = { myPos.x / 20 * 20, myPos.y / 20 * 20 };

	if (otherPos.x < leftTop.x || otherPos.x > leftTop.x + 19)
	{
		return false;
	}

	if (otherPos.y < leftTop.y || otherPos.y > leftTop.y + 19)
	{
		return false;
	}

	return true;
}
