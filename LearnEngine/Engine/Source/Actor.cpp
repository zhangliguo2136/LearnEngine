#include "Actor.h"
TActor::TActor(const std::string& Name)
	:ActorName(Name)
{

}

void TActor::SetActorTransform(const TTransform& NewTransform)
{
}

TTransform TActor::GetActorTransform() const
{
	return TTransform();
}

