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

private:
	TMatrix View = TMatrix::Identity;
	TMatrix Proj = TMatrix::Identity;

	TMatrix PrevViewProj = TMatrix::Identity;
};
