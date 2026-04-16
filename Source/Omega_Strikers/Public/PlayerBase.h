// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Omega_Strikers/SM/OSImpactReceiver.h"
#include "PlayerBase.generated.h"

UCLASS()
class OMEGA_STRIKERS_API APlayerBase : public ACharacter, public IOSImpactReceiver
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
	// UPROPERTY(EditAnywhere, BlueprintReadWrite)
	// TObjectPtr<class HPComponent> HPComp;
	
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
	
	
	// ==================== Stat ==================== (추후 구조체로 관리하거나, csv 관리 시스템 들어오면 수정 가능)
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
	void PlayerMove(const struct FInputActionValue& InputActionValue);
	
	void Ready_CoreHit();
	virtual void Ready_PrimarySkill();
	virtual void Ready_SecondarySkill();
	virtual void Ready_SpecialSkill();
	virtual void Ready_Flip();
	
	void Use_CoreHit();
	virtual void Use_PrimarySkill();
	virtual void Use_SecondarySkill();
	virtual void Use_SpecialSkill();
	virtual void Use_Flip();
	
	// ====== Impact Data Processing Function ======
	virtual void ReceiveImpact_Implementation(const FOSImpactData& ImpactData, AActor* InstigatorActor) override;
	void ApplyKnockback(FVector2D KnockbackDir, float KnockbackPow);
};
