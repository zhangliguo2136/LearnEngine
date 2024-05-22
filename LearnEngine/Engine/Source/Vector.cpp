#include "Vector.h"

#include <math.h>

TVector2f::TVector2f(const float& InX, const float& InY)
	:x(InX), y(InY)
{

}
TVector2f::~TVector2f()
{

}

TVector3f::TVector3f(const float& InX, const float& InY, const float& InZ)
	:x(InX), y(InY), z(InZ)
{

}
TVector3f::~TVector3f()
{

}

void TVector3f::Normalize()
{
	float len = sqrt(x * x + y * y + z * z);

	x = x / len;
	y = y / len;
	z = z / len;
}

float TVector3f::Dot(const TVector3f& v)
{
	return x * v.x + y * v.y + z * v.z;
}

TVector3f TVector3f::Cross(const TVector3f& v)
{
	TVector3f Vec;
	Vec.x = y * v.z - v.y * z;
	Vec.y = v.x * z - x * v.z;
	Vec.z = x * v.y - v.x * y;
	return Vec;
}

const TVector3f TVector3f::Zero = { 0.f, 0.f, 0.f };
const TVector3f TVector3f::One = { 1.f, 1.f, 1.f };
const TVector3f TVector3f::UnitX = { 1.f, 0.f, 0.f };
const TVector3f TVector3f::UnitY = { 0.f, 1.f, 0.f };
const TVector3f TVector3f::UnitZ = { 0.f, 0.f, 1.f };
const TVector3f TVector3f::Up = { 0.f, 1.f, 0.f };
const TVector3f TVector3f::Down = { 0.f, -1.f, 0.f };
const TVector3f TVector3f::Right = { 1.f, 0.f, 0.f };
const TVector3f TVector3f::Left = { -1.f, 0.f, 0.f };
const TVector3f TVector3f::Forward = { 0.f, 0.f, -1.f };
const TVector3f TVector3f::Backward = { 0.f, 0.f, 1.f };

TVector4f::TVector4f()
{
}

TVector4f::TVector4f(TVector3f InXYZ, float InW)
	:x(InXYZ.x), y(InXYZ.y), z(InXYZ.z), w(InW)
{
}
