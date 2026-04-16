// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/Character.h"
#include "PlayerBase.generated.h"

UCLASS()
class OMEGA_STRIKERS_API APlayerBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// ================= Component =================
	
	
	// =================== Input ===================
	
	// IMC
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class UInputMappingContext> IMC_Player;
	
	// 이동
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class UInputAction> IA_Move;
	
	// 스킬
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class UInputAction> IA_Core;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class UInputAction> IA_Primary;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class UInputAction> IA_Secondary;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class UInputAction> IA_Special;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class UInputAction> IA_Flip;
	
	
	// ==================== Stat ====================

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHP = 1125.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurHP;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Power = 50.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Speed = 450.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CoolDownRate = 0;
	
	// ========= Input Processing Function =========
	void PlayerMove(const FInputActionValue& InputActionValue);
	
};
