// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "NiagaraSystem.h"
#include "AimiGlitchOrb.generated.h"

class UProjectileMovementComponent;
class UStaticMeshComponent;
class USphereComponent;
/**
 * 글리치.팝 오브 (Glitch.Pop Orb)
 *
 * ★ 핵심 수학: 폭발 Push Vector = normalize(Target - OrbCenter) × Force
 *
 * - 발사 후 이동하며 점점 커짐 (반지름 선형 증가)
 * - 재시전 또는 최대 거리 도달 시 폭발
 * - 폭발 시 범위 내 적/코어를 오브 중심 → 대상 방향으로 밀어냄
 * - 캐릭터와 빔 연결 (시각적)
 */
UCLASS()
class OMEGA_STRIKERS_API AAimiGlitchOrb : public AActor
{
	GENERATED_BODY()

public:
	AAimiGlitchOrb();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// ──────────────────────────────────────────
	//  발사 / 폭발
	// ──────────────────────────────────────────

	// 오너 캐릭터가 호출 - 방향, 오너 전달
	void Launch(AActor* InOwner, const FVector& Direction);

	// 재시전 시 호출 - 즉시 폭발
	void Detonate();

	// 이미 폭발했는지 체크
	bool HasDetonated() const { return bDetonated; }
	
	// 현재 오브 반지름 (조준 UI에서 사용)
	float GetCurrentRadius() const { return CurrentRadius; }

	// ──────────────────────────────────────────
	//  오브 속성
	// ──────────────────────────────────────────

	// 초기 반경
	UPROPERTY(EditDefaultsOnly, Category="Orb")
	float InitRadius = 30.f;

	// 최대 반경
	UPROPERTY(EditDefaultsOnly, Category="Orb")
	float MaxRadius = 100.f;

	// 초당 반경 성장 속도
	UPROPERTY(EditDefaultsOnly, Category="Orb")
	float GrowthRate = 45.f;

	// 이동 속도
	UPROPERTY(EditDefaultsOnly, Category="Orb")
	float OrbSpeed = 1800.f;

	// 최대 이동 거리 (이후 자동 폭발)
	UPROPERTY(EditDefaultsOnly, Category="Orb")
	float MaxTravelDistance = 1200.f;

	// ──────────────────────────────────────────
	//  폭발 속성
	// ──────────────────────────────────────────

	// 폭발 범위 = 현재 오브의 반지름 * 배수
	UPROPERTY(EditDefaultsOnly, Category="Explosion")
	float ExplosionRadiusMultiplier = 2.5f;

	// 코어 밀치기
	UPROPERTY(EditDefaultsOnly, Category="Explosion")
	float CoreKnockback = 1430.f;

	// 플레이어 밀치기
	UPROPERTY(EditDefaultsOnly, Category="Explosion")
	float PlayerKnockback = 237.f;

	// 피해량
	UPROPERTY(EditDefaultsOnly, Category="Explosion")
	float Damage = 213.f;

	// 폭발 이펙트
	UPROPERTY(EditDefaultsOnly, Category="Explosion")
	TObjectPtr<UNiagaraSystem> DetonateVFX;

protected:
	// ──────────────────────────────────────────
	//  Components
	// ──────────────────────────────────────────

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> OrbMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

private:
	// 발사 원점 (최대 거리 판정용)
	FVector LaunchOrigin;

	// 현재 반지름
	float CurrentRadius;

	// 이동 방향
	FVector MoveDirection;

	// 폭발 완료 체크
	bool bDetonated = false;

	// 오너 캐릭터 참조
	UPROPERTY()
	TObjectPtr<AActor> OwnerCharacter;

	// 현재까지 이동 거리
	float TraveledDistance = 0.f;

	/**
	 * ★ 폭발 로직 — Push Vector 계산
	 *   PushDir = normalize(TargetPos - OrbCenter)
	 *   Force   = BasePower × (1 - dist/explosionRadius × 0.4)
	 */
	void ExecuteExplosion();

	// 반지름 업데이트 ( 콜리전 + 메쉬 스케일 )
	void UpdateRadius(float NewRadius);
}; 
