// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Omega_Strikers/SM/OSType.h"
#include "Asher_Special_Projectile.generated.h"

// 히트 이벤트 델리게이트
DECLARE_DELEGATE_TwoParams(FOnSpecialProjectileHit, FVector  /* HitLocation */, FVector /*Direction*/);

class AAsher_Special_Shield;

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
	
public:
	UPROPERTY(VisibleAnywhere)
	USceneComponent* Root;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UStaticMeshComponent* Mesh1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UStaticMeshComponent* Mesh2;
	
	UPROPERTY(VisibleAnywhere)
	class UBoxComponent* CollisionComp;
	
	FVector MoveDirection = FVector::ZeroVector;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings")
	float Speed = 1200.f;
	
	// 공격자 팀
	EOSTeam OwnerTeam = EOSTeam::None;
	
	// 중복 히트 방지
	TSet<AActor*> HitActors;
	
	// 히트 이벤트
	FOnSpecialProjectileHit OnHit;
	
	// 충돌시 생성되는 방패
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AAsher_Special_Shield> ShieldClass;
public:
	void Init(const FVector& Dir, EOSTeam InTeam);
	
	UFUNCTION()
	void OnHitOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
	
	// 사운드 shield 충돌시  
	UPROPERTY(EditDefaultsOnly, Category="SFX")
	class USoundBase* SpecialHitSFX;
	
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayHitSFX(FVector Location);
};
