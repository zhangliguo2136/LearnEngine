#pragma once

#include <filesystem>
#include <string>

class TFileSystem
{
public:
	static std::string EngineDir()
	{
		std::string EngineDir = "Engine/";

		return EngineDir;
	};
};
