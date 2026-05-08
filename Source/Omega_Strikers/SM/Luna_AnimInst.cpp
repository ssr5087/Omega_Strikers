// Fill out your copyright notice in the Description page of Project Settings.


#include "Luna_AnimInst.h"

#include "Luna.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Omega_Strikers/Omega_Strikers.h"

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
		bIsProcessingPrimary = Luna->bPrimaryAnimTrans;
		bIsProcessingSecondary = Luna->bSecondaryAnimTrans;
		bIsProcessingSpecial = Luna->bSpecialAnimTrans;
	}
}

void ULuna_AnimInst::AnimNotify_PrimarySpawn()
{
	LOG_SM_E(TEXT("스폰 노티파이까지 잘 온 거냐? 클라도 오면 2개 뜨는 거냐?"));
	Luna->SpawnPrimaryRocket();
}

void ULuna_AnimInst::AnimNotify_PrimarySpawnEnd()
{
	LOG_SM_E(TEXT("종료 노티파이까지 잘 온 거냐? 클라도 오면 2개 뜨는 거냐?"));
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
