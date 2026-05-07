// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EXPOrb.generated.h"

UCLASS()
class OMEGA_STRIKERS_API AEXPOrb : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AEXPOrb();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

public:
	// 충돌
	UPROPERTY(VisibleAnywhere)
	class USphereComponent* CollisionComp;
	
	// 메시 (선택)
	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* Mesh;
	
	// 지급 경험치
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 EXPAmount = 50;

	void SetSpawnPoint(class AEXPSpawnPoint* InSpawnPoint);
	
	// 오버랩 함수
	UFUNCTION()
	void OnOverlapBegin(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
	);

protected:
	void ConfigureCollision();
	void TryCollect(class APlayerBase* Player);
	void PollNearbyPlayers();

	UPROPERTY()
	TObjectPtr<class AEXPSpawnPoint> SpawnPoint;
	
};
