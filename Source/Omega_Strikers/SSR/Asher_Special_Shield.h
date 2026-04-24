// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Asher_Special_Shield.generated.h"

UCLASS()
class OMEGA_STRIKERS_API AAsher_Special_Shield : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AAsher_Special_Shield();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UStaticMeshComponent* Mesh1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UStaticMeshComponent* Mesh2;
	
	
	
	// 데미지 타이머
	FTimerHandle DamageTimer;
	float DamageInterval = 0.25f;
	
	// 맞은 대상 기록
	TSet<AActor*> HitActors;
	
	// 방패 이동 관련
	FVector MoveDirection = FVector::ZeroVector;
	float MoveSpeed = 200.f;
	
	// 데미지 관련
	float CenterDamage = 50.f;
	float EdgeDamage = 50.f;
	
	float PlayerKnockback = 200.f;
	float CoreKnockback = 500.f;
	
	float CenterRadius = 100.f;
	
	void Init(const FVector& InDirection);
	
	void ApplyDamage();
	
	bool IsCenter(AActor* Target);
};
