// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "AimiFirewallSentry.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
class UStaticMeshComponent;

/**
 * 방화벽 파수꾼 (Firewall Sentry)
 *
 * - 고정 위치에 터렛 설치
 * - 지정된 방향으로 기술 억제 방화벽 투사체 연사
 * - 각 화살은 처음 적중한 적만 타격 (관통 X)
 * - 일정 시간 후 자동 소멸
 * ★ 네트워크:
 *   - bReplicates=true: 서버에서 스폰 → 클라이언트 자동 복제
 *   - FireProjectile: 서버에서 판정 → Multicast로 VFX 동기화
 */
UCLASS()
class OMEGA_STRIKERS_API AAimiFirewallSentry : public AActor
{
	GENERATED_BODY()

public:
	AAimiFirewallSentry();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// 터렛 초기화 - 발사 방향, 오너 설정
	void Initialize(AActor* IsOwner, const FVector& FireDirection);

	// ──────────────────────────────────────────
	//  터렛 속성
	// ──────────────────────────────────────────

	// 터렛 지속 시간 (초)
	UPROPERTY(EditDefaultsOnly, Category="Sentry")
	float Duration = 6.f;

	// 발사 간격 (초)
	UPROPERTY(EditDefaultsOnly, Category="Sentry")
	float FireInterval = 0.6f;

	// 투사체 속도
	UPROPERTY(EditDefaultsOnly, Category="Sentry")
	float ProjectileSpeed = 2200.f;

	// 투사체 사거리
	UPROPERTY(EditDefaultsOnly, Category="Sentry")
	float ProjectileRange = 1500.f;

	// 투사체 1발당 코어 밀치기
	UPROPERTY(EditDefaultsOnly, Category="Sentry|Damage")
	float ProjectileCoreKnockback = 865.f;

	// 투사체 1발당 플레이어 밀치기
	UPROPERTY(EditDefaultsOnly, Category="Sentry|Damage")
	float ProjectilePlayerKnockback = 175.f;

	// 투사체 1발당 피해
	UPROPERTY(EditDefaultsOnly, Category="Sentry|Damage")
	float ProjectileDamage = 175.f;

	// 투사체 BP 클래스 (없으면 라인트레이스로 대체)
	UPROPERTY(EditDefaultsOnly, Category="Sentry")
	TSubclassOf<AActor> ProjectileClass;

	// 터렛 체력 (파괴 가능)
	UPROPERTY(EditDefaultsOnly, Category="Sentry")
	float SentryHP = 300.f;

	// ──────────────────────────────────────────
	//  VFX 에셋
	// ──────────────────────────────────────────

	// 포탑 소환 이펙트
	UPROPERTY(EditDefaultsOnly, Category="VFX")
	TObjectPtr<UNiagaraSystem> SpawnVFX;

	// 포탑 감지 범위 링 (루프)
	UPROPERTY(EditDefaultsOnly, Category="VFX")
	TObjectPtr<UNiagaraSystem> DetectRingVFX;

	// 발사 머즐플래시
	UPROPERTY(EditDefaultsOnly, Category="VFX")
	TObjectPtr<UNiagaraSystem> MuzzleFlashVFX;

	// 라인트레이스 히트 임팩트
	UPROPERTY(EditDefaultsOnly, Category="VFX")
	TObjectPtr<UNiagaraSystem> HitImpactVFX;

	// 발사 궤적 빔 (라인트레이스 시각화)
	UPROPERTY(EditDefaultsOnly, Category="VFX")
	TObjectPtr<UNiagaraSystem> BeamTrailVFX;

	// 소멸 이펙트
	UPROPERTY(EditDefaultsOnly, Category="VFX")
	TObjectPtr<UNiagaraSystem> DestroyVFX;

	// Multicast RPC — 발사 VFX를 모든 클라이언트에서 재생
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_FireVFX(FVector Start, FVector End, bool bHit, FVector HitPoint);

	// Multicast RPC — 소멸 VFX
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_DestroyVFX(FVector Location, FRotator Rotation);
	
protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> SentryMesh;

	// VFX 컴포넌트
	UPROPERTY(VisibleAnywhere, Category="VFX")
	TObjectPtr<UNiagaraComponent> DetectRingComp;
	
private:
	FVector FireDir;
	float ElapsedTime = 0.f;
	float FireTimer = 0.f;
	float CurrentHP;
	bool bInitialized = false;

	UPROPERTY()
	TObjectPtr<AActor> OwnerCharacter;

	// 투사체 발사 or 라인트레이스 히트
	void FireProjectile();
	
	// 발사 시 VFX 스폰
	void SpawnFireVFX(const FVector& Start, const FVector& End, bool bHit, const FVector& HitPoint);
};
