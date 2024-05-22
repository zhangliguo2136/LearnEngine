#pragma once

class TVector2f
{
public:
	TVector2f() {}
	TVector2f(const float& x, const float& y);
	~TVector2f();
public:
	float x = 0, y = 0;
};

class TVector3f
{
public:
	TVector3f() {}
	TVector3f(const float& x, const float& y, const float& z);
	~TVector3f();

public:
	void Normalize();
	float Dot(const TVector3f& v);
	TVector3f Cross(const TVector3f& v);

public:
	TVector3f operator*(float s);
	TVector3f operator+=(const TVector3f& v);

public:
	float x = 0, y = 0, z = 0;

public:
	static const TVector3f Zero;
	static const TVector3f One;
	static const TVector3f UnitX;
	static const TVector3f UnitY;
	static const TVector3f UnitZ;
	static const TVector3f Up;
	static const TVector3f Down;
	static const TVector3f Right;
	static const TVector3f Left;
	static const TVector3f Forward;
	static const TVector3f Backward;
};

class TVector4f 
{
public:
	TVector4f();
	TVector4f(TVector3f InXYZ, float InW);
public:
	float x = 0, y = 0, z = 0, w = 0;
};