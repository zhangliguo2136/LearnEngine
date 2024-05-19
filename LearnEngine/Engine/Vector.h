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
	float x = 0, y = 0, z = 0;
};