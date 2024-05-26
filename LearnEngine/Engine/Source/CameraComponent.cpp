#include "CameraComponent.h"
#include <iostream>

TCameraComponent::TCameraComponent()
{
	this->SetLens(0.25f * TMath::PI, 960.f/ 540.f, 0.1f, 100.f);
}

TCameraComponent::~TCameraComponent()
{

}

void TCameraComponent::SetPrevViewProj(const TMatrix& VP)
{
	PrevViewProj = VP;
}

TMatrix TCameraComponent::GetPrevViewProj() const
{
	return PrevViewProj;
}

TMatrix TCameraComponent::GetView() const
{
	return View;
}

TMatrix TCameraComponent::GetProj() const
{
	return Proj;
}

void TCameraComponent::SetLens(float InFovY, float InAspect, float InNearZ, float InFarZ)
{
	FovY = InFovY;
	Aspect = InAspect;
	NearZ = InNearZ;
	FarZ = InFarZ;

	Proj = TMatrix::CreatePerspectiveFieldOfView(FovY, Aspect, NearZ, FarZ);
	printf("%f, %f, %f, %f\n", FovY, Aspect, NearZ, FarZ);
}

void TCameraComponent::MoveRight(float Distance)
{
	WorldTransform.Location += Right * Distance;
}

void TCameraComponent::MoveForward(float Distance)
{
	WorldTransform.Location += Look * Distance;
}

void TCameraComponent::MoveUp(float Distance)
{
	WorldTransform.Location = Up * Distance;
}

void TCameraComponent::Pitch(float Degrees)
{
	float Radians = TMath::DegreesToRadians(Degrees);

	// Rotate up and look vector about the right vector.
	TMatrix R = TMatrix::CreateFromAxisAngle(Right, Radians);

	Up = R.TransformNormal(Up);
	Look = R.TransformNormal(Look);
}

void TCameraComponent::RotateY(float Degrees)
{
	float Radians = TMath::DegreesToRadians(Degrees);

	// Rotate the basis vectors about the world y-axis.
	TMatrix R = TMatrix::CreateRotationY(Radians);

	Right = R.TransformNormal(Right);
	Up = R.TransformNormal(Up);
	Look = R.TransformNormal(Look);
}

void TCameraComponent::UpdateViewMatrix()
{
	Look.Normalize();
	Up = Look.Cross(Right);
	Up.Normalize();

	// Up, Look already ortho-normal, so no need to normalize cross product.
	Right = Up.Cross(Look);

	// Fill in the view matrix entries.
	float x = - WorldTransform.Location.Dot(Right);
	float y = - WorldTransform.Location.Dot(Up);
	float z = - WorldTransform.Location.Dot(Look);

	View(0, 0) = Right.x;
	View(1, 0) = Right.y;
	View(2, 0) = Right.z;
	View(3, 0) = x;

	View(0, 1) = Up.x;
	View(1, 1) = Up.y;
	View(2, 1) = Up.z;
	View(3, 1) = y;

	View(0, 2) = Look.x;
	View(1, 2) = Look.y;
	View(2, 2) = Look.z;
	View(3, 2) = z;

	View(0, 3) = 0.0f;
	View(1, 3) = 0.0f;
	View(2, 3) = 0.0f;
	View(3, 3) = 1.0f;
}
