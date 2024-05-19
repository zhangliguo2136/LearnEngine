#include "Vector.h"

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