#pragma once
#include "Vector.h"
#include "Matrix.h"
#include "MyMath.h"

struct TRotator
{
	float Roll;
	float Pitch;
	float Yaw;

	TRotator()
		:Roll(0.0f), Pitch(0.0f), Yaw(0.0f)
	{}

	TRotator(float InRoll, float InPitch, float InYaw)
		:Roll(InRoll), Pitch(InPitch), Yaw(InYaw)
	{}

	static const TRotator Zero;
};

class TTransform
{
public:
	TTransform()
	{
		Location = TVector3f::Zero;
		Rotation = TRotator::Zero;
		Scale = TVector3f::One;
	}

	TMatrix GetTransformMatrix() const
	{
		TMatrix S = TMatrix::CreateScale(Scale);
		TMatrix R = TMatrix::CreateFromYawPitchRoll(Rotation.Yaw * TMath::PI / 180.0f, Rotation.Pitch * TMath::PI / 180.0f, Rotation.Roll * TMath::PI / 180.0f);
		TMatrix T = TMatrix::CreateTranslation(Location);

		return S * R * T;
	}

public:
	TVector3f Location;
	TRotator Rotation;
	TVector3f Scale;
};