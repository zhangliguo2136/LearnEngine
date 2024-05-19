#pragma once
#include <string>

class TMaterial;
class TMaterialInstance
{
public:
	TMaterialInstance(TMaterial* InParent, const std::string& InName);

public:
	std::string Name;
	TMaterial* Parent = nullptr;
};