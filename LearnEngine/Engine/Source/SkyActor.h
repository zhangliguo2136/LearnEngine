#pragma once

#include "Actor.h"

class TMeshComponent;
class TSkyActor : public TActor
{
public:
	TSkyActor(const std::string& Name);
	~TSkyActor();

	void SetMaterialInstance(std::string MaterialInstanceName);

public:
	TMeshComponent* MeshComponent = nullptr;
};
