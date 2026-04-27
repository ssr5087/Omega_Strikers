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
	
	// ================= Controller =================
	
	// 안 쓰게 되면 지울 것!!!!!!!!!
	UPROPERTY()
	class AOSPlayerController* myPC;
	
	
	// ================= Component =================
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Component")
	TObjectPtr<class UHPComponent> HPComp;
	
	
	// =================== Input ===================
	
	// IMC
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	TObjectPtr<class UInputMappingContext> IMC_Player;
	
	// 이동
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	TObjectPtr<class UInputAction> IA_Move;
	
	// 스킬
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	TObjectPtr<class UInputAction> IA_Core;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	TObjectPtr<class UInputAction> IA_Primary;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	TObjectPtr<class UInputAction> IA_Secondary;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	TObjectPtr<class UInputAction> IA_Special;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	TObjectPtr<class UInputAction> IA_Flip;
	
	// 마우스 커서 자체의 위치
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Input")
	FVector MouseCursorLoc;
	
	// 마우스 커서 조준 방향
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Input")
	FVector2D CursorDir = FVector2D::ZeroVector;
	
	
	// ==================== Stat ==================== (추후 구조체로 관리하거나, csv 관리 시스템 들어오면 수정 가능)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stat|Base")
	EOSTeam TeamSide = EOSTeam::None;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stat|Base")
	float MaxHP = 1125.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stat|Base")
	float Power = 50.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stat|Base")
	float Speed = 450.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stat|Base")
	float CoolDownRate = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stat|Knockback")
	float KnockbackRatio = 1;
	
	bool bCoreHitCoolDown = false;
	
	float CoreHitCool = 0.9f;
	float PrimarySkillCool = 0.f;
	float SecondaryCool = 0.f;
	float SpecialCool = 0.f;
	
	
	// ========= Input Processing Function =========
	
	virtual void PlayerMove(const struct FInputActionValue& InputActionValue);
	
	virtual void Ready_CoreHit();
	virtual void Ready_PrimarySkill();
	virtual void Ready_SecondarySkill();
	virtual void Ready_SpecialSkill();
	virtual void Ready_Flip();
	
	virtual void Use_CoreHit();
	virtual void Use_PrimarySkill();
	virtual void Use_SecondarySkill();
	virtual void Use_SpecialSkill();
	virtual void Use_Flip();
	
	
	// ======= Other Processing Function =======
	
	// Impact Data 처리 함수(인터페이스 함수)
	virtual bool ReceiveImpact_Implementation(const FOSImpactData& ImpactData, AActor* InstigatorActor) override;
	void ApplyKnockback(FVector2D KnockbackDir, float KnockbackPow);
	
	// 넉백 계수 관리 함수
	void KnockbackIncrease();
	void KnockbackDecrease();
	
	// Knockout 상태 처리 함수 및 리스폰 처리 함수 등등
	// 넉백 상태일 때 벽과의 충돌을 잠시 꺼둠
	// 그 벽 뒤에 있는 박스와의 충돌이 일어나면 넉아웃으로 판정 - 이펙트 나오고 캐릭터 visibility를 false로 설정 
	// 플레이어 위치를 리스폰 위치로 옮기고, 넉백 계수 등 스탯 값 초기값으로 세팅(체력은 컴포넌트 통해서)
	// 10초 타이머 끝나면 이펙트와 함께 캐릭터 visibility true로 재설정
};
