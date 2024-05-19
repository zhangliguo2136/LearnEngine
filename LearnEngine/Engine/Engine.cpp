#include "Engine.h"

#include "World.h"
#include "Render.h"
#include "D3DRHI.h"

TEngine::TEngine(TWorld* InWorld)
{
	World.reset(InWorld);

	Render = std::make_unique<TRender>();
}

void TEngine::Update(float dt)
{
	World->Update(dt);
	Render->Draw(dt);
}
