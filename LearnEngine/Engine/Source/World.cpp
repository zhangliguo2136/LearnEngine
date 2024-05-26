#include "World.h"
#include "CameraActor.h"

TWorld::TWorld()
{
}

void TWorld::BeginFrame()
{
}

void TWorld::Update(float dt)
{
	for (auto& Actor : Actors)
	{
		Actor->Tick(dt);
	}
}

void TWorld::EndFrame()
{
	for (auto& Actor : Actors)
	{
		Actor->SetActorPrevTransform(Actor->GetActorTransform());
	}

	auto CameraActors = this->GetAllActorsOfClass<TCameraActor>();
	for (auto CameraActor : CameraActors)
	{
		TMatrix View = CameraActor->GetCameraComponent()->GetView();
		TMatrix Proj = CameraActor->GetCameraComponent()->GetProj();
		CameraActor->GetCameraComponent()->SetPrevViewProj(View * Proj);
	}
}
