#include "World.h"

TWorld::TWorld()
{
}

void TWorld::Update(float dt)
{
	for (auto& Actor : Actors)
	{
		Actor->Tick(dt);
	}
}
