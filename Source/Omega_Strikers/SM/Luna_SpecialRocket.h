// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OSType.h"
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
	
	// 폭발 이펙트
	UPROPERTY(EditDefaultsOnly, Category="FX")
	TObjectPtr<class UNiagaraSystem> ExplosionVFX;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Rocket")
	float Luna_Power = 50.0f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Rocket")
	FOSImpactData SpecialNearImpactData;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Rocket")
	FOSImpactData SpecialFarImpactData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Rocket")
	TObjectPtr<class ALuna> OwnerActorRef = nullptr;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Rocket")
	EOSTeam TeamSide = EOSTeam::Red;
	
	void InitRocket(AActor* InOwnerActor);
	
	UFUNCTION()
	void OnSpecialRocketOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(ReplicatedUsing=OnRep_VFX)
	FVector ActorLocation;
	
	UFUNCTION(NetMulticast, Reliable)
	void OnRep_VFX();
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
};
