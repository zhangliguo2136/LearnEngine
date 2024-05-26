#include "DirectionalLightActor.h"

TDirectionalLightActor::TDirectionalLightActor(const std::string& Name)
	:TLightActor(Name, ELightType::DirectionalLight)
{
	RootComponent = AddComponent<TComponent>();
}

TDirectionalLightActor::~TDirectionalLightActor()
{
}

void TDirectionalLightActor::SetActorTransform(const TTransform& NewTransform)
{
	TActor::SetActorTransform(NewTransform);

	SetLightDirection(NewTransform.Rotation);
}

TVector3f TDirectionalLightActor::GetLightDirection() const
{
	return Direction;
}

void TDirectionalLightActor::SetLightDirection(TRotator Rotation)
{
	TMatrix R = TMatrix::CreateFromYawPitchRoll(Rotation.Yaw * TMath::PI / 180.0f, Rotation.Pitch * TMath::PI / 180.0f, Rotation.Roll * TMath::PI / 180.0f);
	Direction = R.TransformNormal(TVector3f::Up);
}
