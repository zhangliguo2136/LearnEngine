#include "Matrix.h"

const TMatrix TMatrix::Identity = { 1.f, 0.f, 0.f, 0.f,
								  0.f, 1.f, 0.f, 0.f,
								  0.f, 0.f, 1.f, 0.f,
								  0.f, 0.f, 0.f, 1.f };

TMatrix TMatrix::CreateScale(TVector3f Sacle)
{
	return TMatrix::Identity;
}

TMatrix TMatrix::CreateFromYawPitchRoll(float yaw, float pitch, float roll)
{
	return TMatrix::Identity;
}

TMatrix TMatrix::CreateTranslation(TVector3f Location)
{
	return TMatrix::Identity;
}

TMatrix TMatrix::Transpose()
{
	return TMatrix::Identity;
}

TMatrix& TMatrix::operator*(const TMatrix& M) noexcept
{
	// TODO: 在此处插入 return 语句
	return *this;
}

TMatrix& TMatrix::operator*=(const TMatrix& M) noexcept
{
	// TODO: 在此处插入 return 语句
	return *this;
}
