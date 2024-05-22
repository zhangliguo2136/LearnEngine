#pragma once

#include "Component.h"

class TCameraComponent : public TComponent
{
public:
	TCameraComponent();
	~TCameraComponent();

public:
	void SetPrevViewProj(const TMatrix& VP);
	TMatrix GetPrevViewProj() const;

	TMatrix GetView() const;
	TMatrix GetProj() const;

	void SetLens(float InFovY, float InAspect, float InNearZ, float InFarZ);

	void MoveRight(float Distance);
	void MoveForward(float Distance);
	void MoveUp(float Distance);

	void Pitch(float Degrees);
	void RotateY(float Degrees);

	void UpdateViewMatrix();

private:
	TMatrix View = TMatrix::Identity;
	TMatrix Proj = TMatrix::Identity;

	TMatrix PrevViewProj = TMatrix::Identity;

	// Camera coordinate system with coordinates relative to world space.
	TVector3f Right = { 1.0f, 0.0f, 0.0f };
	TVector3f Up = { 0.0f, 1.0f, 0.0f };
	TVector3f Look = { 0.0f, 0.0f, 1.0f };

	// Cache frustum properties.
	float NearZ = 0.0f;
	float FarZ = 0.0f;
	float Aspect = 0.0f;
	float FovY = 0.0f;
};
