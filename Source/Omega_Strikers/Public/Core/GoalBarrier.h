// Fill out your copyright notice in the Description page of Project Settings.

// 에디터에서 골대 앞에 직접 배치
// 사다리꼴 메시 (원작에서 찾아서 사용)
// 코어가 맞으면 HP 감소 -> 0이 되면 파괴 -> 모든 배리어가 파괴되면 골 게이트 오픈

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GoalBarrier.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBarrierDestroyed, AGoalBarrier*, Barrier);

UCLASS()
class OMEGA_STRIKERS_API AGoalBarrier : public AActor
{
	GENERATED_BODY()

public:
	AGoalBarrier();

	// ───────────────────────────────────────────
	// 비주얼
	// ───────────────────────────────────────────
	
	// 배리어 메시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Barrier")
	UStaticMeshComponent* BarrierMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barrier")
	UStaticMesh* BarrierMeshAsset;
	
	// ───────────────────────────────────────────
	// 게임플레이
	// ───────────────────────────────────────────
	
	// 소속 팀 (0=Blue/좌측, 1=Red/우측)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Barrier|GamePlay")
	int32 TeamIndex = 0;
	
	// 파괴에 필요한 코어 히트 횟수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barrier|GamePlay")
	int32 MaxHP = 1;
	
	// 현재 HP
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Barrier|GamePlay", Replicated)
	int32 CurrentHP = 1;
	
	// 파괴되었는지 체크
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Barrier|GamePlay", Replicated)
	bool bIsDestroyed = false;
	
	// ───────────────────────────────────────────
	// 콜리전
	// ───────────────────────────────────────────
	
	// ture: 메시 자체 콜리전 사용
	// false: Simple Collision 사용
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barrier|Collision")
	bool bUseComplexCollision = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barrier|Collision", meta=(ClampMin = "0.0", ClampMax = "1.0"))
	float Restitution = 0.85f;
	
	// ───────────────────────────────────────────
	// 이벤트
	// ───────────────────────────────────────────
	
	// 배리어 파괴 시 브로드캐스트 (CoreArena에서 수신)
	UPROPERTY(BlueprintAssignable, Category = "Barrier|Events")
	FOnBarrierDestroyed OnBarrierDestroyed;
	
	// ───────────────────────────────────────────
	// 함수
	// ───────────────────────────────────────────
	
	// 코어에 의한 데미지 (서버에서 호출)
	UFUNCTION(BlueprintCallable, Category = "Barrier")
	void HitByCore();
	
	// 배리어 리셋 (라운드 시작)
	UFUNCTION(BlueprintCallable, Category = "Barrier")
	void ResetBarrier();
	
	// 파괴 연출 (블루프린트에서 오버라이드 가능)
	UFUNCTION(BlueprintNativeEvent, Category = "Barrier")
	void PlayDestroyFX();
	
	// 히트 연출
	UFUNCTION(BlueprintNativeEvent, Category = "Barrier")
	void PlayHitFX();
	
protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void NotifyHit(class UPrimitiveComponent* MyComp, AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

private:
	UPROPERTY()
	class UPhysicalMaterial* BarrierPhysMat;
	
	void SetupCollision();
};
