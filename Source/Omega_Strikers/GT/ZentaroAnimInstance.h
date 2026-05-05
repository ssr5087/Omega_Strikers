// Fill out your copyright notice in the Description page of Project Settings.
// ZentaroAnimInstance.h
// 천전천승 — 젠타로 전용 AnimInstance
// ABP_Zentaro의 Parent Class로 지정

#pragma once

#include "CoreMinimal.h"

#include "ZentaroAnimInstance.generated.h"

UCLASS()
class OMEGA_STRIKERS_API UZentaroAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;

	// ─────────────────────────────────────────────
	// 로코모션
	// ─────────────────────────────────────────────

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float Speed = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	bool bIsDashing = false;

	// ─────────────────────────────────────────────
	// 스킬 상태
	// ─────────────────────────────────────────────

	// Strike 충전 횟수 (0 ~ 10), ABP 레이어 전환에 활용
	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	int32 StrikeChargeCount = 0;

	// Flip Energy (0 ~ 100), 100일 때 360도 폭발 애니 분기
	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	float FlipEnergy = 0.f;

	// 오니의 분노 발동 중
	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	bool bIsOniRage = false;

	// 섀도우 스템 (E) 순간이동 중
	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	bool bIsShadowStepping = false;

	// ─────────────────────────────────────────────
	// 몽타주
	// ─────────────────────────────────────────────

	// 기본 Strike (미충전)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montage")
	TObjectPtr<UAnimMontage> StrikeMontage;

	// Strike 충전 최대 (10회) — 강화 타격
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montage")
	TObjectPtr<UAnimMontage> StrikeChargedMontage;
 
	// Q 산산 — Phase1 즉발 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montage")
	TObjectPtr<UAnimMontage> DoShatteredPhase1Montage;
 
	// Q 산산 — Phase2 (0.2초 딜레이 후)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montage")
	TObjectPtr<UAnimMontage> DoShatteredPhase2Montage;
 
	// E 섀도우 스텝 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montage")
	TObjectPtr<UAnimMontage> IawaseMontage;
 
	// R 오니의 분노 — 시작
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montage")
	TObjectPtr<UAnimMontage> OniRageStartMontage;
 
	// R 오니의 분노 — 약타 루프 (SetTimer 8회)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montage")
	TObjectPtr<UAnimMontage> OniRageHitMontage;
	
	// R 오니의 분노 — 최종 폭발
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montage")
	TObjectPtr<UAnimMontage> OniRageFinishMontage;
 
	// Flip — 미충전 대시
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montage")
	TObjectPtr<UAnimMontage> FlipMontage;
 
	// Flip — Energy 100% 폭발
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montage")
	TObjectPtr<UAnimMontage> FlipExplosionMontage;
 
	// ─────────────────────────────────────────────
	// 몽타주 재생 헬퍼
	// ─────────────────────────────────────────────
 
	UFUNCTION(BlueprintCallable, Category = "Zentaro|Anim")
	void PlayStrike(bool bCharged);
 
	UFUNCTION(BlueprintCallable, Category = "Zentaro|Anim")
	void PlaySansanPhase1();
 
	UFUNCTION(BlueprintCallable, Category = "Zentaro|Anim")
	void PlaySansanPhase2();
 
	UFUNCTION(BlueprintCallable, Category = "Zentaro|Anim")
	void PlayIawase();
 
	UFUNCTION(BlueprintCallable, Category = "Zentaro|Anim")
	void PlayOniRageStart();
 
	UFUNCTION(BlueprintCallable, Category = "Zentaro|Anim")
	void PlayOniRageHit();
 
	UFUNCTION(BlueprintCallable, Category = "Zentaro|Anim")
	void PlayOniRageFinish();
 
	UFUNCTION(BlueprintCallable, Category = "Zentaro|Anim")
	void PlayFlip(bool bExplosion);
 
private:
	UPROPERTY()
	TObjectPtr<class AZentaro> OwnerCharacter;
};
