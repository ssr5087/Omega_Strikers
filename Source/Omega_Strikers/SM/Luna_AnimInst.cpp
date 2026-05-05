// Fill out your copyright notice in the Description page of Project Settings.


#include "Luna_AnimInst.h"

#include "Luna.h"
#include "GameFramework/CharacterMovementComponent.h"

void ULuna_AnimInst::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	Luna = Cast<ALuna>(TryGetPawnOwner());
}

void ULuna_AnimInst::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	
	if (Luna)
	{
		bIsMoving = Luna->GetVelocity().Size2D() > 1.f;
		bIsProcessingCoreHit = Luna->bCoreHitCoolDown; // 얘 가라로 해놓은 거니까 나중에 맞게 다시 처리할 필요 있음
		bIsProcessingPrimary = Luna->bIsProcessingPrimary;
		bIsProcessingSecondary = Luna->bIsProcessingSecondary;
		bIsProcessingSpecial = Luna->bIsProcessingSpecial;
	}
}

void ULuna_AnimInst::AnimNotify_PrimarySpawn()
{
	Luna->SpawnPrimaryRocket();
}

void ULuna_AnimInst::AnimNotify_PrimarySpawnEnd()
{
	Luna->End_PrimarySkill();
}

void ULuna_AnimInst::AnimNotify_SpecialSpawn()
{
	Luna->SpawnSpecialRocket();
}

void ULuna_AnimInst::AnimNotify_SpecialSpawnEnd()
{
	Luna->End_SpecialSkill();
}
