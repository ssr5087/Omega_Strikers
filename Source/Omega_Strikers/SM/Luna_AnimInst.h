// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Luna_AnimInst.generated.h"

/**
 * 
 */
UCLASS()
class OMEGA_STRIKERS_API ULuna_AnimInst : public UAnimInstance
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
	class ALuna* Luna;
	
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Luna|Move")
	bool bIsMoving = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Luna|Skill")
	bool bIsProcessingSecondary = false;
};
