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
};

struct TObjectConstants
{
	TMatrix World = TMatrix::Identity;
	TMatrix PrevWorld = TMatrix::Identity;
};