#include "Actor.h"

TActor::TActor(const std::string& Name)
	:ActorName(Name)
{

}

void TActor::SetActorTransform(const TTransform& NewTransform)
{
	RootComponent->SetWorldTransform(NewTransform);
}

TTransform TActor::GetActorTransform() const
{
	return RootComponent->GetWorldTransform();
}

void TActor::SetActorPrevTransform(const TTransform& PrevTransform)
{
	RootComponent->SetPrevWorldTransform(PrevTransform);
}

TTransform TActor::GetActorPrevTransform() const
{
	return RootComponent->GetPrevWorldTransform();
}

