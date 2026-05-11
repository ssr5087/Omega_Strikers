// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Asher_AnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class OMEGA_STRIKERS_API UAsher_AnimInstance : public UAnimInstance
{
	GENERATED_BODY()

	
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float Speed = 0.f;
	
	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsSecondaryDashing = false;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsPrimaryAttacking = false;
	
	// 스킬 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Montage")
	TObjectPtr<class UAnimMontage> StrikeMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Montage")
	TObjectPtr<class UAnimMontage> PrimaryMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Montage")
	TObjectPtr<class UAnimMontage> SecondaryMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Montage")
	TObjectPtr<class UAnimMontage> SpecialMontage;
	
	// 재생 함수
	void PlayPrimary();
	void PlaySecondary();
	void PlaySpecial();
	
	
private:
	// 소유 캐릭터 캐시
	UPROPERTY()
	TObjectPtr<class AAsher> OwnerCharacter;
};
