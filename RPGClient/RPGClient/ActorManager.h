#pragma once

#include "Actor.h"

class ActorManager
{
public:
	static Actor& RegisterActor(const int32 id, const int16 race,
		const int16 x, const int16 y, const int16 level,
		const int32 hp, const int32 hpMax,
		const String& name, const int32 exp = 0);

	static void RemoveActor(const int32 id);

	static void MoveActor(const int32 id, const int16 x, const int16 y);

	static void RenderActors(const Point& myPos);

	static String GetActorName(const int32 id);

private:
	static bool isInSameArea(const Point& myPos, const Point& otherPos);

private:
	static std::unordered_map<int32, Actor> sActors;
};

