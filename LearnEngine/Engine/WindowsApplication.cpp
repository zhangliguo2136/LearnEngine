#include "WindowsApplication.h"

void TWindowsApplication::Tick(TEngine* Engine)
{
	Engine->Update(0.f);
}

bool TWindowsApplication::IsQuit()
{
	return false;
}

bool TWindowsApplication::Initialize()
{
	return false;
}

bool TWindowsApplication::Finalize()
{
	return false;
}
