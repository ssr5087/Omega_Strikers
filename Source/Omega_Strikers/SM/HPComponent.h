// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HPComponent.generated.h"

DECLARE_DELEGATE(FHPBecomeNegative);
DECLARE_DELEGATE(FHPBecomePositive);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class OMEGA_STRIKERS_API UHPComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UHPComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	// ============== Delegate ==============
	
	FHPBecomeNegative OnHPBecomeNegative;
	FHPBecomePositive OnHPBecomePositive;
	
	
	// ============== Variable ==============
	
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = HPComponent)
	float MaxHP = 1125.f;
	UPROPERTY(Replicated, BlueprintReadOnly, Category = HPComponent)
	float CurHP;
	
	UPROPERTY(Replicated, BlueprintReadOnly, Category = HPComponent)
	bool bIsStaggered = false;
	
	FTimerHandle HealTimer;
	
	
	// ============== Function ==============
	// 님 서버임 클라임?
	UFUNCTION(BlueprintCallable, Category = HPComponent)
	bool IsServer() const;
	
	// 캐릭터 현재 체력을 최대 체력으로 초기화
	UFUNCTION(BlueprintCallable, Category = HPComponent)
	void InitializeHP();
	
	// 캐릭터 레벨 업 시 최대 체력 업데이트
	UFUNCTION(BlueprintCallable, Category = HPComponent)
	void UpdateMaxHP(float NewMax);
	
	// 캐릭터 피격 시 현재 체력 감소
	UFUNCTION(BlueprintCallable, Category = HPComponent)
	void ApplyDamage(float DamageAmount);
	
	// 캐릭터 피격 후 일정 시간 동안 계속 힐
	UFUNCTION(BlueprintCallable, Category = HPComponent)
	void DotHeal();
	
	// 캐릭터 오브 획득 또는 팀원 힐 스킬 받았을 때의 힐 적용
	UFUNCTION(BlueprintCallable, Category = HPComponent)
	void ApplyHeal(float HealAmount);
	
	
	// ============== Networking ==============
	
	// 변수 등록
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
