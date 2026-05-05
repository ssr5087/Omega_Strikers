// Fill out your copyright notice in the Description page of Project Settings.


#include "ZentaroAnimInstance.h"
#include "Zentaro.h"
#include "GameFramework/CharacterMovementComponent.h"

void UZentaroAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	OwnerCharacter = Cast<AZentaro>(GetOwningActor());
}

void UZentaroAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);
 
	if (!OwnerCharacter) return;
 
	const UCharacterMovementComponent* MoveComp =
		OwnerCharacter->GetCharacterMovement();
 
	// ── 로코모션 ──────────────────────────────────────
	Speed     = OwnerCharacter->GetVelocity().Size2D();
	bIsInAir  = MoveComp ? MoveComp->IsFalling() : false;
	bIsDashing = OwnerCharacter->bIsDashing;
 
	// ── 스킬 상태 ─────────────────────────────────────
	// 젠타로 캐릭터에서 해당 변수들을 BlueprintReadOnly 또는 public으로 노출 필요
	StrikeChargeCount  = OwnerCharacter->StrikeChargeCount;   // 0~10
	FlipEnergy         = OwnerCharacter->FlipEnergy;           // 0~100
	bIsOniRage         = OwnerCharacter->bIsOniRage;
	bIsShadowStepping  = OwnerCharacter->bIsIawaseTeleporting;
}

void UZentaroAnimInstance::PlayStrike(bool bCharged)
{
}

void UZentaroAnimInstance::PlaySansanPhase1()
{
}

void UZentaroAnimInstance::PlaySansanPhase2()
{
}

void UZentaroAnimInstance::PlayShadowStep()
{
}

void UZentaroAnimInstance::PlayOniRageStart()
{
}

void UZentaroAnimInstance::PlayOniRageHit()
{
}

void UZentaroAnimInstance::PlayOniRageFinish()
{
}

void UZentaroAnimInstance::PlayFlip(bool bExplosion)
{
}
