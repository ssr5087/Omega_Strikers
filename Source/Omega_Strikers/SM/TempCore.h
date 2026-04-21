// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OSImpactReceiver.h"
#include "GameFramework/Actor.h"
#include "TempCore.generated.h"

UCLASS()
class OMEGA_STRIKERS_API ATempCore : public AActor, public IOSImpactReceiver
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATempCore();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Distance = FVector::ZeroVector;
	float Speed = 0;
	
	void ReceiveImpact_Implementation(const FOSImpactData& ImpactData, AActor* InstigatorActor);
};
