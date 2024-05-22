#include "Matrix.h"
#include "Quaternion.h"

#include <math.h>
#include <algorithm>

const TMatrix TMatrix::Identity = { 1.f, 0.f, 0.f, 0.f,
								  0.f, 1.f, 0.f, 0.f,
								  0.f, 0.f, 1.f, 0.f,
								  0.f, 0.f, 0.f, 1.f };

TMatrix TMatrix::CreateScale(TVector3f Scale)
{
	TMatrix Mat = TMatrix::Identity;
	Mat(0, 0) = Scale.x;
	Mat(1, 1) = Scale.y;
	Mat(2, 2) = Scale.z;

	return Mat;
}

TMatrix TMatrix::CreateFromYawPitchRoll(float Yaw, float Pitch, float Roll)
{
	float cp = cosf(Pitch);
	float sp = sinf(Pitch);

	float cy = cosf(Yaw);
	float sy = sinf(Yaw);

	float cr = cosf(Roll);
	float sr = sinf(Roll);

	TMatrix M = TMatrix::Identity;
	M.m[0][0] = cr * cy + sr * sp * sy;
	M.m[0][1] = sr * cp;
	M.m[0][2] = sr * sp * cy - cr * sy;
	M.m[0][3] = 0.0f;

	M.m[1][0] = cr * sp * sy - sr * cy;
	M.m[1][1] = cr * cp;
	M.m[1][2] = sr * sy + cr * sp * cy;
	M.m[1][3] = 0.0f;

	M.m[2][0] = cp * sy;
	M.m[2][1] = -sp;
	M.m[2][2] = cp * cy;
	M.m[2][3] = 0.0f;

	M.m[3][0] = 0.0f;
	M.m[3][1] = 0.0f;
	M.m[3][2] = 0.0f;
	M.m[3][3] = 1.0f;
	return M;
}

TMatrix TMatrix::CreateTranslation(TVector3f Location)
{
	TMatrix Mat = TMatrix::Identity;
	Mat(0, 3) = Location.x;
	Mat(1, 3) = Location.y;
	Mat(2, 3) = Location.z;
	return Mat;
}

TMatrix TMatrix::CreateFromAxisAngle(TVector3f Right, float Radians)
{

	Right.Normalize();
	TQuaternion Quat(Right, Radians);

	TMatrix NewMatrix = TMatrix::RotationByQuat(Quat);

	return TMatrix::Identity;
}

TMatrix TMatrix::CreateRotationY(float Radians)
{
	float fSinAngle = sin(Radians);
	float fCosAngle = cos(Radians);

	TMatrix M = TMatrix::Identity;

	M.m[0][0] = fCosAngle;
	M.m[0][1] = fSinAngle;
	M.m[0][2] = 0.0f;
	M.m[0][3] = 0.0f;

	M.m[1][0] = -fSinAngle;
	M.m[1][1] = fCosAngle;
	M.m[1][2] = 0.0f;
	M.m[1][3] = 0.0f;

	M.m[2][0] = 0.0f;
	M.m[2][1] = 0.0f;
	M.m[2][2] = 1.0f;
	M.m[2][3] = 0.0f;

	M.m[3][0] = 0.0f;
	M.m[3][1] = 0.0f;
	M.m[3][2] = 0.0f;
	M.m[3][3] = 1.0f;

	return M;
}

TMatrix TMatrix::CreatePerspectiveFieldOfView(float FovY, float Aspect, float NearZ, float FarZ)
{
	float q = 1.0f / tan(FovY / 2);

	float A = q / Aspect;
	float B = (NearZ + FarZ) / (NearZ - FarZ);
	float C = (2.0f * NearZ * FarZ) / (NearZ - FarZ);

	TMatrix mat = TMatrix::Identity;
	mat(0, 0) = A;
	mat(1, 1) = q;
	mat(2, 2) = B;
	mat(2, 3) = -1;
	mat(3, 3) = C;
	return mat;
}

TMatrix TMatrix::RotationByQuat(const TQuaternion& quat)
{
	TMatrix Mat = TMatrix::Identity;

	Mat(0, 0) = 1.0f - 2.0f * quat.y * quat.y - 2.0f * quat.z * quat.z;
	Mat(0, 1) = 2.0f * quat.x * quat.y + 2.0f * quat.w * quat.z;
	Mat(0, 2) = 2.0f * quat.x * quat.z - 2.0f * quat.w * quat.y;
	Mat(0, 3) = 0.0f;

	Mat(1, 0) = 2.0f * quat.x * quat.y - 2.0f * quat.w * quat.z;
	Mat(1, 1) = 1.0f - 2.0f * quat.x * quat.x - 2.0f * quat.z * quat.z;
	Mat(1, 2) = 2.0f * quat.y * quat.z + 2.0f * quat.w * quat.x;
	Mat(1, 3) = 0.0f;

	Mat(2, 0) = 2.0f * quat.x * quat.z + 2.0f * quat.w * quat.y;
	Mat(2, 1) = 2.0f * quat.y * quat.z - 2.0f * quat.w * quat.x;
	Mat(2, 2) = 1.0f - 2.0f * quat.x * quat.x - 2.0f * quat.y * quat.y;
	Mat(2, 3) = 0.0f;

	Mat(3, 0) = 0.0f;
	Mat(3, 1) = 0.0f;
	Mat(3, 2) = 0.0f;
	Mat(3, 3) = 1.0f;

	return Mat;
}

TMatrix TMatrix::Transpose()
{
	TMatrix Mat = TMatrix::Identity;
	Mat(0, 0) = m[0][0];
	Mat(0, 1) = m[1][0];
	Mat(0, 2) = m[2][0];
	Mat(0, 3) = m[3][0];

	Mat(1, 0) = m[0][1];
	Mat(1, 1) = m[1][1];
	Mat(1, 2) = m[2][1];
	Mat(1, 3) = m[3][1];

	Mat(2, 0) = m[0][2];
	Mat(2, 1) = m[1][2];
	Mat(2, 2) = m[2][2];
	Mat(2, 3) = m[3][2];

	Mat(3, 0) = m[0][3];
	Mat(3, 1) = m[1][3];
	Mat(3, 2) = m[2][3];
	Mat(3, 3) = m[3][3];

	return Mat;
}

TVector3f TMatrix::TransformNormal(const TVector3f& v)
{
	TVector4f NewVec = (*this) * TVector4f(v, 0.f);

	return TVector3f(NewVec.x, NewVec.y, NewVec.z);
}

TMatrix TMatrix::operator*(const TMatrix& M) noexcept
{
	TMatrix NewMat = TMatrix::Identity;

	NewMat(0, 0) = m[0][0] * M(0, 0) + m[0][1] * M(1, 0) + m[0][2] * M(2, 0) + m[0][3] * M(3, 0);
	NewMat(0, 1) = m[0][0] * M(0, 1) + m[0][1] * M(1, 1) + m[0][2] * M(2, 1) + m[0][3] * M(3, 1);
	NewMat(0, 2) = m[0][0] * M(0, 2) + m[0][1] * M(1, 2) + m[0][2] * M(2, 2) + m[0][3] * M(3, 2);
	NewMat(0, 3) = m[0][0] * M(0, 3) + m[0][1] * M(1, 3) + m[0][2] * M(2, 3) + m[0][3] * M(3, 3);

	NewMat(1, 0) = m[1][0] * M(0, 0) + m[1][1] * M(1, 0) + m[1][2] * M(2, 0) + m[1][3] * M(3, 0);
	NewMat(1, 1) = m[1][0] * M(0, 1) + m[1][1] * M(1, 1) + m[1][2] * M(2, 1) + m[1][3] * M(3, 1);
	NewMat(1, 2) = m[1][0] * M(0, 2) + m[1][1] * M(1, 2) + m[1][2] * M(2, 2) + m[1][3] * M(3, 2);
	NewMat(1, 3) = m[1][0] * M(0, 3) + m[1][1] * M(1, 3) + m[1][2] * M(2, 3) + m[1][3] * M(3, 3);

	NewMat(2, 0) = m[2][0] * M(0, 0) + m[2][1] * M(1, 0) + m[2][2] * M(2, 0) + m[2][3] * M(3, 0);
	NewMat(2, 1) = m[2][0] * M(0, 1) + m[2][1] * M(1, 1) + m[2][2] * M(2, 1) + m[2][3] * M(3, 1);
	NewMat(2, 2) = m[2][0] * M(0, 2) + m[2][1] * M(1, 2) + m[2][2] * M(2, 2) + m[2][3] * M(3, 2);
	NewMat(2, 3) = m[2][0] * M(0, 3) + m[2][1] * M(1, 3) + m[2][2] * M(2, 3) + m[2][3] * M(3, 3);

	NewMat(3, 0) = m[3][0] * M(0, 0) + m[3][1] * M(1, 0) + m[3][2] * M(2, 0) + m[3][3] * M(3, 0);
	NewMat(3, 1) = m[3][0] * M(0, 1) + m[3][1] * M(1, 1) + m[3][2] * M(2, 1) + m[3][3] * M(3, 1);
	NewMat(3, 2) = m[3][0] * M(0, 2) + m[3][1] * M(1, 2) + m[3][2] * M(2, 2) + m[3][3] * M(3, 2);
	NewMat(3, 3) = m[3][0] * M(0, 3) + m[3][1] * M(1, 3) + m[3][2] * M(2, 3) + m[3][3] * M(3, 3);

	return NewMat;
}

TVector4f TMatrix::operator*(const TVector4f& V) noexcept
{
	TVector4f NewV;

	NewV.x = m[0][0] * V.x + m[0][1] * V.y + m[0][2] * V.z + m[0][3] * V.w;
	NewV.y = m[1][0] * V.x + m[1][1] * V.y + m[1][2] * V.z + m[1][3] * V.w;
	NewV.z = m[2][0] * V.x + m[2][1] * V.y + m[2][2] * V.z + m[2][3] * V.w;
	NewV.w = m[3][0] * V.x + m[3][1] * V.y + m[3][2] * V.z + m[3][3] * V.w;

	return NewV;
}