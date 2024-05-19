#pragma once
#include "Actor.h"

class TMeshComponent;

class TStaticMeshActor : public TActor
{
public:
	TStaticMeshActor(const std::string& Name);

	~TStaticMeshActor();

public:
	void SetMeshName(std::string MeshName);

	void SetMaterialInstance(std::string MaterialInstanceName);

private:
	TMeshComponent* StaticMeshComponent = nullptr;
};