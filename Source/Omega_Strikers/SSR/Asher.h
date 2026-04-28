// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterStat.h"
#include "PlayerBase.h"
#include "Asher.generated.h"

UCLASS()
class OMEGA_STRIKERS_API AAsher : public APlayerBase
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAsher();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	virtual void Ready_CoreHit() override;
	virtual void Ready_PrimarySkill() override;
	virtual void Ready_SecondarySkill() override;
	virtual void Ready_SpecialSkill() override;
	virtual void Ready_Flip() override;
	
	virtual void Use_CoreHit() override;
	virtual void Use_PrimarySkill() override;
	virtual void Use_SecondarySkill() override;
	virtual void Use_SpecialSkill() override;
	virtual void Use_Flip() override;
	

	
public:
	// Primary Skill
	// 콤보 상태일때
	bool bIsPrimary_Attacking = false;
	// 맞은 대상 기록 (중복 방지)
	TSet<AActor*> HitActors;
	
	FTimerHandle PrimaryHit2Timer;
	FTimerHandle PrimaryEndTimer;
	
	// boolean 스킬 쿨타임 체크
	bool bPrimary_SkillCoolDown = false;
	
	// 스킬 쿨타임 시간
	float Primary_SkillCool = 4.0f;
	
	// 1타
	void DoPrimaryHit1();
	// 2타
	void DoPrimaryHit2();
	
	// ------------------------------------------------------------
	
	// Special Skill
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AAsher_Special_Projectile> SpecialProjectileClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AAsher_Special_Shield> SpecialShieldClass;
	
	// boolean 스킬 쿨타임 체크
	bool bSpecial_SkillCoolDown = false;
	
	// 스킬 타이머 설정
	FTimerHandle SpecialSkillTimer;
	
	// 스킬 쿨타임 시간
	float Special_SkillCool = 4.0f;
	
	EOSTeam MyTeam = EOSTeam::Blue;
	
	void DoSpecialProjectile();
	void DoSpecialShield(FVector SpawnLocation, FVector Direction);
	
	// ---------------------------------------
	
	// Secondary Skill
	bool bSecondary_SkillCoolDown = false;
	bool bIsSecondary_Dashing = false;

	FTimerHandle SecondarySkillTimer;
	FTimerHandle SecondaryDashTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Secondary")
	float Secondary_SkillCool = 4.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Secondary")
	float Secondary_DashDistance = 700.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Secondary")
	float Secondary_DashDuration = 0.18f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Secondary")
	float Secondary_DashTraceInterval = 0.02f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Secondary")
	float Secondary_HitRadius = 120.0f;
	
	// 데미지
	float Secondary_PlayerDamage = 120.0f;
	float Secondary_PlayerKnockback = 850.0f;
	float Secondary_CoreKnockback = 1500.0f;

	FVector SecondaryDashDirection = FVector::ZeroVector;
	FVector SecondaryLastTraceLocation = FVector::ZeroVector;
	TSet<AActor*> SecondaryHitActors;

	void DoSecondaryDash();
	void DoSecondaryDashTrace();
	void EndSecondaryDash();
};
