#pragma once
#include "Actor.h"
#include "Vector.h"

enum ELightType
{
	None,
	DirectionalLight,
};

class TLightActor : public TActor
{
public:
	TLightActor(const std::string& Name, ELightType InType);

	~TLightActor();
public:
	TVector3f GetLightColor() const
	{
		return Color;
	}

	virtual void SetLightColor(const TVector3f& InColor)
	{
		Color = InColor;
	}

	float GetLightIntensity() const
	{
		return Intensity;
	}

	virtual void SetLightIntensity(float InIntensity)
	{
		Intensity = InIntensity;
	}

private:
	ELightType Type = ELightType::None;

	TVector3f Color = {1.f, 1.f, 1.f};

	float Intensity = 10.0f;
};