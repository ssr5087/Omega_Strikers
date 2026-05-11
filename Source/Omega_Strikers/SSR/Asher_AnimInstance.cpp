// Fill out your copyright notice in the Description page of Project Settings.


#include "Asher_AnimInstance.h"

#include "Asher.h"
#include "GameFramework/CharacterMovementComponent.h"

void UAsher_AnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	OwnerCharacter = Cast<AAsher>(GetOwningActor());
}

void UAsher_AnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	
	if (!OwnerCharacter)
		return; 
	
	const UCharacterMovementComponent* moveComp = OwnerCharacter->GetCharacterMovement();
	
	// Locomotion
	Speed = moveComp ? moveComp->Velocity.Size2D() : OwnerCharacter->GetVelocity().Size2D();
	bIsSecondaryDashing = OwnerCharacter->bIsSecondary_Dashing;
	bIsPrimaryAttacking = OwnerCharacter->bIsPrimary_Attacking;
}

void UAsher_AnimInstance::PlayStrike()
{
	if (StrikeMontage)
	{
		Montage_Play(StrikeMontage);
	}
}

void UAsher_AnimInstance::PlayPrimary()
{
	if (PrimaryMontage)
	{
		Montage_Play(PrimaryMontage);
	}
}


void UAsher_AnimInstance::PlaySecondary()
{
	if (SecondaryMontage)
	{
		Montage_Play(SecondaryMontage);
	}
}

void UAsher_AnimInstance::PlaySpecial()
{
	if (SpecialMontage)
	{
		Montage_Play(SpecialMontage);
	}
}
