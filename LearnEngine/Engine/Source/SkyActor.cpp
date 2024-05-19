#include "SkyActor.h"
#include "MeshComponent.h"

TSkyActor::TSkyActor(const std::string& Name)
	:TActor(Name)
{
	MeshComponent = AddComponent<TMeshComponent>();
	RootComponent = MeshComponent;

	//Mesh
	MeshComponent->SetMeshName("SphereMesh");
}

TSkyActor::~TSkyActor()
{

}

void TSkyActor::SetMaterialInstance(std::string MaterialInstanceName)
{
	MeshComponent->SetMaterialInstance(MaterialInstanceName);
}