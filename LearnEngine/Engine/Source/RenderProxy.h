#pragma once
#include <stdint.h>

#include "Vector.h"
#include "Matrix.h"

struct TMaterialConstant
{
	TVector3f EmissiveColor;
	uint32_t ShadingModel;
};

struct TPassConstants
{
	TMatrix View = TMatrix::Identity;
	TMatrix Proj = TMatrix::Identity;
	TVector3f EyePosW = { 0.0f, 0.0f, 0.0f };
};

struct TObjectConstants
{
	TMatrix World = TMatrix::Identity;
	TMatrix PrevWorld = TMatrix::Identity;
};

struct TLightParameters
{
	TVector3f Color;
	float Intensity;
	UINT LightType;
	TVector3f Direction;
};