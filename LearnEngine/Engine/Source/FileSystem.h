#pragma once

#include <string>

class TFileSystem
{
public:
	static std::string EngineDir()
	{
		//std::string EngineDir = "../Engine/";

		std::string EngineDir = std::string(SOLUTION_DIR) + "/Engine/";

		return EngineDir;
	};
};
