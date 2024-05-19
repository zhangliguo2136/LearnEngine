#pragma once
#include "Vector.h"

class TMatrix
{

public:
	TMatrix(float m00, float m01, float m02, float m03,
		float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23,
		float m30, float m31, float m32, float m33)
		:_11(m00), _12(m01), _13(m02), _14(m03),
		_21(m10), _22(m11), _23(m12), _24(m13),
		_31(m20), _32(m21), _33(m22), _34(m23),
		_41(m30), _42(m31), _43(m32), _44(m33)
	{

	}
public:
	union
	{
		struct
		{
			float _11, _12, _13, _14;
			float _21, _22, _23, _24;
			float _31, _32, _33, _34;
			float _41, _42, _43, _44;
		};
		float m[4][4];
	};

public:
	static TMatrix CreateScale(TVector3f Sacle);
	static TMatrix CreateFromYawPitchRoll(float yaw, float pitch, float roll);
	static TMatrix CreateTranslation(TVector3f Location);

	TMatrix Transpose();

	// ������
	TMatrix& operator* (const TMatrix& M) noexcept;
	TMatrix& operator*= (const TMatrix& M) noexcept;

public:
	// Constants
	static const TMatrix Identity;
};