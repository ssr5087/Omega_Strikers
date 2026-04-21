// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Luna_PrimaryRocket.generated.h"

UCLASS()
class OMEGA_STRIKERS_API ALuna_PrimaryRocket : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ALuna_PrimaryRocket();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
