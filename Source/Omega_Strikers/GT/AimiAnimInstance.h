// Fill out your copyright notice in the Description page of Project Settings.
// AimiAnimInstance.h
// 천전천승 — 아이미 전용 AnimInstance
// ABP_Aimi의 Parent Class로 지정

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "AimiAnimInstance.generated.h"

UCLASS()
class OMEGA_STRIKERS_API UAimiAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;

	// ─────────────────────────────────────────────
	// 로코모션
	// ─────────────────────────────────────────────

	// 이동 속도 (BlendSpace 가로축)
	UPROPERTY(BlueprintReadOnly, Category="Locomotion")
	float Speed = 0.f;

	// 대시 중인지 체크
	UPROPERTY(BlueprintReadOnly, Category="Locomotion")
	bool bIsDashing = false;

	// ─────────────────────────────────────────────
	// 스킬 상태 (ABP 레이어 전환용)
	// ─────────────────────────────────────────────

	// 글리치팝 오브 에이밍 중
	UPROPERTY(BlueprintReadOnly, Category="Skill")
	bool bIsAimingOrb = false;

	// 백도어 포탑 설치 중
	UPROPERTY(BlueprintReadOnly, Category="Skill")
	bool bIsPlacingSentry = false;

	// 사이퍼 서지 발동 중
	UPROPERTY(BlueprintReadOnly, Category="Skill")
	bool bIsSurging = false;

	// ─────────────────────────────────────────────
	// 몽타주 슬롯 (에디터 ABP에서 SlotNode로 연결)
	// ─────────────────────────────────────────────

	// 기본 공격 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Montage")
	TObjectPtr<UAnimMontage> StrikeMontage;

	// 글리치 팝 발사 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Montage")
	TObjectPtr<UAnimMontage> GlitchOrbMontage;

	// 백도어 포탑 배치 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Montage")
	TObjectPtr<UAnimMontage> BackdoorMontage;

	// 사이퍼 서지 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Montage")
	TObjectPtr<UAnimMontage> CipherSurgeMontage;

	// Flip(대시) 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Montage")
	TObjectPtr<UAnimMontage> FlipMontage;

	// ─────────────────────────────────────────────
	// 몽타주 재생 헬퍼 (캐릭터 클래스에서 호출)
	// ─────────────────────────────────────────────

	UFUNCTION(BlueprintCallable, Category="Aimi|Anim")
	void PlayStrike();

	UFUNCTION(BlueprintCallable, Category="Aimi|Anim")
	void PlayGlitchOrb();

	UFUNCTION(BlueprintCallable, Category="Aimi|Anim")
	void PlayBackdoor();

	UFUNCTION(BlueprintCallable, Category="Aimi|Anim")
	void PlayCipherSurge();

	UFUNCTION(BlueprintCallable, Category="Aimi|Anim")
	void PlayFlip();

private:
	// 소유 캐릭터 캐시
	UPROPERTY()
	TObjectPtr<class AAimi> OwnerCharacter;
};
