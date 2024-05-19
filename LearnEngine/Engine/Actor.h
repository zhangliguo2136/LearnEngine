#pragma once
#include <string>
#include <vector>
#include <memory>
#include "Component.h"

class TActor
{
public:
	TActor(const std::string& Name);

	virtual ~TActor() {}

public:
	virtual void Tick(float DeltaTime) {}

public:
	template<typename T>
	std::vector<T*> GetComponentsOfClass()
	{
		std::vector<T*> Result;
		for (const auto& Component : Components)
		{
			T* ComponentOfClass = dynamic_cast<T*>(Component.get());
			if (ComponentOfClass)
			{
				Result.push_back(ComponentOfClass);
			}
		}

		return Result;
	}

protected:
	std::string ActorName;

	TComponent* RootComponent = nullptr;
	std::vector<std::unique_ptr<TComponent>> Components;
};