#pragma once
#include "LightActor.h"
#include "Vector.h"

class TDirectionalLightActor : public TLightActor
{
public:
	TDirectionalLightActor(const std::string& Name);

	~TDirectionalLightActor();
public:
	virtual void SetActorTransform(const TTransform& NewTransform) override;

	TVector3f GetLightDirection() const;

	void SetLightDirection(TRotator Rotation);

private:
	TVector3f Direction;
};
