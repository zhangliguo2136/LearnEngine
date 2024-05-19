#pragma once
#include <string>
#include <vector>
#include <memory>

#include "Component.h"
#include "Transform.h"

class TActor
{
public:
	TActor(const std::string& Name);

	virtual ~TActor() {}

public:
	virtual void Tick(float DeltaTime) {}

public:
	template<typename T>
	T* AddComponent()
	{
		auto NewComponent = std::make_unique<T>();
		T* Result = NewComponent.get();
		Components.push_back(std::move(NewComponent));

		return Result;
	}

	std::vector<TComponent*> GetComponets()
	{
		std::vector<TComponent*> Result;

		for (const auto& Component : Components)
		{
			Result.push_back(Component.get());
		}

		return Result;
	}
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


	TComponent* GetRootComponent() const
	{
		return RootComponent;
	}

	virtual void SetActorTransform(const TTransform& NewTransform);
	TTransform GetActorTransform() const;

	void SetActorPrevTransform(const TTransform& PrevTransform);
	TTransform GetActorPrevTransform() const;

protected:
	std::string ActorName;

	TComponent* RootComponent = nullptr;
	std::vector<std::unique_ptr<TComponent>> Components;
};