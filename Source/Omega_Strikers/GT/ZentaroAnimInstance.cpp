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
	bIsDashing = OwnerCharacter->bIsDashing;
 
	// ── 스킬 상태 ─────────────────────────────────────
	// 젠타로 캐릭터에서 해당 변수들을 BlueprintReadOnly 또는 public으로 노출 필요
	StrikeChargeCount  = OwnerCharacter->StrikeChargeCount;   // 0~10
	FlipEnergy         = OwnerCharacter->FlipEnergy;           // 0~100
	bIsOniRage         = OwnerCharacter->bIsOniRage;
	bIsShadowStepping  = OwnerCharacter->bIsIawaseTeleporting;
}
// ──────────────────────────────────────────────────────
// 몽타주 재생 헬퍼
// ──────────────────────────────────────────────────────
 
void UZentaroAnimInstance::PlayStrike(bool bCharged)
{
	UAnimMontage* Target = bCharged ? StrikeChargedMontage : StrikeMontage;
	if (Target && !Montage_IsPlaying(Target))
		Montage_Play(Target);
}
 
void UZentaroAnimInstance::PlaySansanPhase1()
{
	if (DoShatteredPhase1Montage)
		Montage_Play(DoShatteredPhase1Montage);
}
 
void UZentaroAnimInstance::PlaySansanPhase2()
{
	if (DoShatteredPhase2Montage)
		Montage_Play(DoShatteredPhase2Montage);
}
 
void UZentaroAnimInstance::PlayIawase()
{
	if (IawaseMontage)
		Montage_Play(IawaseMontage);
}
 
void UZentaroAnimInstance::PlayOniRageStart()
{
	if (OniRageStartMontage)
		Montage_Play(OniRageStartMontage);
}
 
void UZentaroAnimInstance::PlayOniRageHit()
{
	if (OniRageHitMontage)
		Montage_Play(OniRageHitMontage);
}
 
void UZentaroAnimInstance::PlayOniRageFinish()
{
	if (OniRageFinishMontage)
		Montage_Play(OniRageFinishMontage);
}
 
void UZentaroAnimInstance::PlayFlip(bool bExplosion)
{
	UAnimMontage* Target = bExplosion ? FlipExplosionMontage : FlipMontage;
	if (Target)
		Montage_Play(Target);
}
 