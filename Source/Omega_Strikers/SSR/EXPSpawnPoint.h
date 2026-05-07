// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EXPSpawnPoint.generated.h"

UCLASS()
class OMEGA_STRIKERS_API AEXPSpawnPoint : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AEXPSpawnPoint();

	bool CanSpawnOrb() const;
	void RegisterOrb(class AEXPOrb* InOrb);
	void ClearOrb(const class AEXPOrb* InOrb = nullptr);

	// 이미 Orb가 있는지 체크용
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="EXP")
	bool bHasOrb = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="EXP")
	TObjectPtr<class AEXPOrb> SpawnedOrb;
};
