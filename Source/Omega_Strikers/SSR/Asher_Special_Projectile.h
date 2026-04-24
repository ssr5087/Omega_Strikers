// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Asher_Special_Projectile.generated.h"

// 히트 이벤트 델리게이트
DECLARE_DELEGATE_TwoParams(FOnSpecialProjectileHit, FVector  /* HitLocation */, FVector /*Direction*/);


UCLASS()
class OMEGA_STRIKERS_API AAsher_Special_Projectile : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AAsher_Special_Projectile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
