#pragma once

#include <string>
#include <vector>

#include "Actor.h"

class TWorld
{
public:
	TWorld();

	void Update(float dt);

public:
public:
	template<typename T>
	T* AddActor(std::string& Name)
	{
		auto NewActor = std::make_unique<T>(Name);
		T* Result = NewActor.get();
		Actors.push_back(std::move(NewActor));
		return Result;
	}
	std::vector<TActor*> GetActors()
	{
		std::vector<TActor*> Result;

		for (const auto& Actor : Actors)
		{
			Result.push_back(Actor.get());
		}

		return Result;
	}

	template<typename T>
	std::vector<T*> GetAllActorsOfClass()
	{
		std::vector<T*> Result;
		for (const auto& Actor : Actors)
		{
			T* ActorOfClass = dynamic_cast<T*>(Actor.get());
			if (ActorOfClass)
			{
				Result.push_back(ActorOfClass);
			}
		}
		return Result;
	}

private:
	std::vector<std::unique_ptr<TActor>> Actors;
};