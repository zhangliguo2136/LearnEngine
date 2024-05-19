#include <iostream>

#include "Sample.h"
#include "../Engine/WindowsApplication.h"
#include "../Engine/Engine.h"

int main()
{
	printf("Enter Sample World!");

	TSampleWorld* World = new TSampleWorld(); 
	TEngine* Engine = new TEngine(World);
	TWindowsApplication* App = new TWindowsApplication();

	App->Initialize();

	while (!App->IsQuit())
	{
		App->Tick(Engine);
	}

	App->Finalize();

	return 0;
}