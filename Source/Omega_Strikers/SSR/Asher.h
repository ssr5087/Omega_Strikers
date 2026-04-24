// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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
	
	// boolean 스킬 쿨타임 체크
	bool bSpecial_SkillCoolDown = false;
	
	// 스킬 타이머 설정
	FTimerHandle SpecialSkillTimer;
	
	// 스킬 쿨타임 시간
	float Special_SkillCool = 4.0f;
	
	void DoSpecialProjectile();
	void DoSpecialShield();
};
