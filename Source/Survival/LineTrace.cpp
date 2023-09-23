// Fill out your copyright notice in the Description page of Project Settings.


#include "LineTrace.h"

// Sets default values for this component's properties
ULineTrace::ULineTrace()
{
}

// Called when the game starts
void ULineTrace::BeginPlay()
{
	Super::BeginPlay();

}

FHitResult ULineTrace::LineTraceSingle(FVector start, FVector end)
{
	FHitResult hitresult;
	FCollisionObjectQueryParams CollisionParams;
	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActor(GetOwner());

	if (GetWorld()->LineTraceSingleByObjectType( OUT hitresult,start,end, CollisionParams,CollisionQueryParams))
	{
		return hitresult;
	}
	else
		return hitresult;
}

FHitResult ULineTrace::LineTraceSingle(FVector start, FVector end, bool ShowDebugLine)
{
	FHitResult Actor = LineTraceSingle(start, end);

	if(ShowDebugLine)
	DrawDebugLine(GetWorld(), start, end, FColor::Red, false, 3.0f, 0, 5.0f);
	return Actor;
}




