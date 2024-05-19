#include "DirectionalLightActor.h"

TDirectionalLightActor::TDirectionalLightActor(const std::string& Name)
	:TLightActor(Name, ELightType::DirectionalLight)
{
}

TDirectionalLightActor::~TDirectionalLightActor()
{
}

TVector3f TDirectionalLightActor::GetLightDirection() const
{
	return Direction;
}
