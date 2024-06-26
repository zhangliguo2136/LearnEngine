#pragma once
#pragma once

#include "Actor.h"
#include "CameraComponent.h"

class TCameraActor : public TActor
{
public:
	TCameraActor(const std::string& Name);
	~TCameraActor();

	TCameraComponent* GetCameraComponent();

	virtual void Tick(float dt) override;

private:
	TCameraComponent* CameraComponent = nullptr;
};
