#pragma once
#include <string>

#include "Component.h"
#include "MaterialInstance.h"

class TMeshComponent :public TComponent
{
public:
	void SetMeshName(const std::string& InMeshName);

	std::string GetMeshName() const {
		return MeshName;
	};

	void SetMaterialInstance(std::string MaterialInstanceName);
	TMaterialInstance* GetMaterialInstance() { return MaterialInstance; }

private:
	std::string MeshName;
	TMaterialInstance* MaterialInstance;
};