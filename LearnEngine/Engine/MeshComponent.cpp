#include "MeshComponent.h"
#include "MaterialRepository.h"

void TMeshComponent::SetMeshName(const std::string& InMeshName)
{
	MeshName = InMeshName;
}

void TMeshComponent::SetMaterialInstance(std::string MaterialInstanceName)
{
	MaterialInstance = TMaterialRepository::Get().GetMaterialInstance(MaterialInstanceName);
}
