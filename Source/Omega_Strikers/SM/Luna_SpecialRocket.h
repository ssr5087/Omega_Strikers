// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Luna_SpecialRocket.generated.h"

UCLASS()
class OMEGA_STRIKERS_API ALuna_SpecialRocket : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ALuna_SpecialRocket();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Luna|Components")
	TObjectPtr<class UBoxComponent> BoxComp;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Luna|Components")
	TObjectPtr<class UStaticMeshComponent> RocketMesh;
	
	UFUNCTION()
	void OnSpecialRocketOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

};
