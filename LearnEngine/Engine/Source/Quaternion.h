#pragma once

#include "Vector.h"

/*
	ËÄÔªÊý
*/
class TQuaternion
{
public:
	TQuaternion(float inX, float inY, float inZ, float inW);
	TQuaternion(const TVector3f& axis, float angle);

public:
	float x = 0.f, y = 0.f, z = 0.f, w = 0.f;
};