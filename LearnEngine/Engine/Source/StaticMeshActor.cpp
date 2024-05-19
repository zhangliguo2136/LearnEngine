#include "StaticMeshActor.h"
#include "MeshComponent.h"

TStaticMeshActor::TStaticMeshActor(const std::string& Name)
	:TActor(Name)
{
	StaticMeshComponent = AddComponent<TMeshComponent>();

	RootComponent = StaticMeshComponent;
}

TStaticMeshActor::~TStaticMeshActor()
{
}

void TStaticMeshActor::SetMeshName(std::string MeshName)
{
	StaticMeshComponent->SetMeshName(MeshName);
}

void TStaticMeshActor::SetMaterialInstance(std::string MaterialInstanceName)
{
	StaticMeshComponent->SetMaterialInstance(MaterialInstanceName);
}
