// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpawnEXPOrb.generated.h"

UCLASS()
class OMEGA_STRIKERS_API ASpawnEXPOrb : public AActor
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	ASpawnEXPOrb();

	// 오브 있는지 체크용
	bool bHasOrb = false;
	
};
