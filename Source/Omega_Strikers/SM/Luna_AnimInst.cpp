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
		bIsProcessingSecondary = Luna->bIsProcessingSecondary;
	}
}
