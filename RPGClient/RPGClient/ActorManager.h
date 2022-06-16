#pragma once

#include "Actor.h"

class ActorManager
{
public:
	static Actor& RegisterActor(const int32 id, const int16 x, const int16 y,
		const String& name);

	static void RemoveActor(const int32 id);

	static void MoveActor(const int32 id, const int16 x, const int16 y);

	static void RenderActors();

	static String GetActorName(const int32 id);

private:
	static std::unordered_map<int32, Actor> sActors;
};

