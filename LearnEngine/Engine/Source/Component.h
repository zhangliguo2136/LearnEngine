#pragma once
#include "Transform.h"

class TComponent
{
public:
	TComponent() {}
	virtual ~TComponent() {}

public:
	virtual void SetWorldLocation(const TVector3f& Location)
	{
		WorldTransform.Location = Location;
	}

	TVector3f GetWorldLocation() const
	{
		return WorldTransform.Location;
	}

	virtual void SetWorldRotation(const TRotator& Rotation)
	{
		WorldTransform.Rotation = Rotation;
	}

	TRotator GetWorldRotation() const
	{
		return WorldTransform.Rotation;
	}

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