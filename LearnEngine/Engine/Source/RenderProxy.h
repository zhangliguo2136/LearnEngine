#pragma once
#include <stdint.h>

#include "Vector.h"
#include "Matrix.h"

struct TMaterialConstant
{
	uint32_t ShadingModel;
	TVector3f EmissiveColor;
};

struct TPassConstants
{
	TMatrix View = TMatrix::Identity;
	TMatrix Proj = TMatrix::Identity;
	TMatrix ViewProj = TMatrix::Identity;
	TMatrix PrevViewProj = TMatrix::Identity;
	TVector3f EyePosW = { 0.0f, 0.0f, 0.0f };
};

struct TObjectConstants
{
	TMatrix World = TMatrix::Identity;
	TMatrix PrevWorld = TMatrix::Identity;
};

struct TLightCommon
{
	UINT LightCount;
};

struct TLightParameters
{
	TVector3f Color;
	float Intensity;
	UINT LightType;
	TVector3f Direction;
};