// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Asher_Special_Shield.generated.h"

class AAPlayerBase;

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
	// Component
	UPROPERTY(VisibleAnywhere)
	USceneComponent* Root;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UStaticMeshComponent* Mesh1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UStaticMeshComponent* Mesh2;
	
	UPROPERTY(Editanywhere, BlueprintReadWrite)
	class UBoxComponent* CollisionComp;
	
	// 데미지 타이머
	FTimerHandle DamageTimer;
	FTimerHandle LifeTimer;
	
	UPROPERTY(EditAnywhere)
	float DamageInterval = 0.33f;
	
	// 맞은 대상 기록
	TSet<AActor*> HitActors;
	
	// 이동
	FVector MoveDirection = FVector::ZeroVector;

	UPROPERTY(EditAnywhere)
	float MoveSpeed = 200.f;
	
	// 데이터셋 
	class APlayerBase* OwnerPlayer = nullptr;
	
	UPROPERTY(EditAnywhere)
	float CenterRadius = 100.f;
	
	// 초기화 Projectile 방향, 회전값 전달
	void Init(const FVector& InDirection, const FRotator& InRotation);
	
	void ApplyDamage();
	
	bool IsCenter(AActor* Target);
	
	
};
