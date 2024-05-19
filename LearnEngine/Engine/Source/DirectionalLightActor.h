#pragma once
#include "LightActor.h"
#include "Vector.h"

class TDirectionalLightActor : public TLightActor
{
public:
	TDirectionalLightActor(const std::string& Name);

	~TDirectionalLightActor();
public:
	TVector3f GetLightDirection() const;

private:
	TVector3f Direction;
};
