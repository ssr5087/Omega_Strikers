// Fill out your copyright notice in the Description page of Project Settings.


#include "EXPSpawnPoint.h"

#include "EXPOrb.h"


// Sets default values
AEXPSpawnPoint::AEXPSpawnPoint()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

bool AEXPSpawnPoint::CanSpawnOrb() const
{
	return !bHasOrb && !IsValid(SpawnedOrb);
}

void AEXPSpawnPoint::RegisterOrb(AEXPOrb* InOrb)
{
	SpawnedOrb = InOrb;
	bHasOrb = IsValid(InOrb);
}

void AEXPSpawnPoint::ClearOrb(const AEXPOrb* InOrb)
{
	if (InOrb && SpawnedOrb != InOrb)
	{
		return;
	}

	SpawnedOrb = nullptr;
	bHasOrb = false;
}

