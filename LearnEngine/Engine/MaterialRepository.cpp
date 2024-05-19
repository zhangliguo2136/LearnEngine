#include "MaterialRepository.h"

TMaterialRepository& TMaterialRepository::Get()
{
	static TMaterialRepository Instance;
	return Instance;
}

void TMaterialRepository::Load()
{

}

void TMaterialRepository::Unload()
{
	MaterialMap.clear();
	MaterialInstanceMap.clear();
}

TMaterialInstance* TMaterialRepository::GetMaterialInstance(const std::string& MaterialInstanceName) const
{
	TMaterialInstance* Result = nullptr;

	auto Iter = MaterialInstanceMap.find(MaterialInstanceName);
	if (Iter != MaterialInstanceMap.end())
	{
		Result = Iter->second.get();
	}

	return Result;
}

