#pragma once
#include <string>
#include "Material.h"

class TMaterialInstance
{
public:
	TMaterialInstance(TMaterial* InParent, const std::string& InName);

	void UpdateConstants(TD3DRHI* D3DRHI);
public:
	std::string Name;
	TMaterial* Parent = nullptr;

	std::shared_ptr<TD3DResource> MaterialConstantsRef = nullptr;

	std::unordered_map<std::string, std::string> TextureMap;
};