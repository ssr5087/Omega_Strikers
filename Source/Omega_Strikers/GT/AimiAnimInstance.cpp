// Fill out your copyright notice in the Description page of Project Settings.


#include "AimiAnimInstance.h"
#include "Aimi.h"
#include "GameFramework/CharacterMovementComponent.h"

// ──────────────────────────────────────────────────────
// 초기화
// ──────────────────────────────────────────────────────

void UAimiAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    OwnerCharacter = Cast<AAimi>(GetOwningActor());
}

// ──────────────────────────────────────────────────────
// 매 프레임 갱신 — 로코모션 & 스킬 상태 폴링
// ──────────────────────────────────────────────────────

void UAimiAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
    Super::NativeUpdateAnimation(DeltaTime);

    if (!OwnerCharacter) return;

    const UCharacterMovementComponent* moveComp = OwnerCharacter->GetCharacterMovement();

    // 로코모션
    Speed = OwnerCharacter->GetVelocity().Size2D();

    // 대시: 아이미 캐릭터에서 bIsDashing 플래그 노출 필요
    bIsDashing = OwnerCharacter->bIsDashing;

    // 스킬 상태
    // 아이미 캐릭터에서 해당 플래그들을 노출해줘야 함
    bIsAimingOrb = OwnerCharacter->bIsAimingOrb;
    bIsPlacingSentry = OwnerCharacter->bIsPlacingSentry;
    bIsSurging = OwnerCharacter->bIsSurging;
}

// ──────────────────────────────────────────────────────
// 몽타주 재생 헬퍼
// ──────────────────────────────────────────────────────

void UAimiAnimInstance::PlayStrike()
{
    if (StrikeMontage && !Montage_IsPlaying(StrikeMontage))
        Montage_Play(StrikeMontage);
    
    
}

void UAimiAnimInstance::PlayGlitchOrb()
{
    if (GlitchOrbMontage && !Montage_IsPlaying(GlitchOrbMontage))
        Montage_Play(GlitchOrbMontage);
}

void UAimiAnimInstance::PlayCyberSwipe()
{
    if (CyberSwipeMontage && !Montage_IsPlaying(CyberSwipeMontage))
        Montage_Play(CyberSwipeMontage);
}

void UAimiAnimInstance::PlayPlaceSentry()
{
    if (SentryMontage && !Montage_IsPlaying(SentryMontage))
        Montage_Play(SentryMontage);
}

void UAimiAnimInstance::PlayFlip()
{
    if (FlipMontage && !Montage_IsPlaying(FlipMontage))
        Montage_Play(FlipMontage);
}
