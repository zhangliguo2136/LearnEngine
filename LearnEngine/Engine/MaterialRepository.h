#pragma once
#include <string>

#include "Material.h"
#include "MaterialInstance.h"

class TMaterialRepository
{
public:
	static TMaterialRepository& Get();

	void Load();
	void Unload();

	TMaterialInstance* GetMaterialInstance(const std::string& MaterialInstanceName) const;

private:
	std::unordered_map<std::string, std::unique_ptr<TMaterial>> MaterialMap;
	std::unordered_map<std::string, std::unique_ptr<TMaterialInstance>> MaterialInstanceMap;
};