#pragma once
#include "Transform.h"

class TComponent
{
public:
	TComponent() {}
	virtual ~TComponent() {}

public:
	void SetWorldTransform(const TTransform& Transform)
	{
		WorldTransform = Transform;
	}
	TTransform GetWorldTransform() const
	{
		return WorldTransform;
	}

	void SetPrevWorldTransform(const TTransform& Transform)
	{
		PrevWorldTransform = Transform;
	}
	TTransform GetPrevWorldTransform() const
	{
		return PrevWorldTransform;
	}

protected:
	TTransform WorldTransform;
	TTransform PrevWorldTransform;
};