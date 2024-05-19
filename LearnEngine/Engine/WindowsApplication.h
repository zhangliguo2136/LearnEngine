#pragma once

#include "Engine.h"

class TWindowsApplication
{
public:
	void Tick(TEngine* Engine);
	bool IsQuit();

	bool Initialize();
	bool Finalize();
};