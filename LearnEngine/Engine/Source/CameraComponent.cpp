#include "CameraComponent.h"

TCameraComponent::TCameraComponent()
{
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
