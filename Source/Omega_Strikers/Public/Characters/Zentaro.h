// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerBase.h"

#include "Zentaro.generated.h"

/**
 * 젠타로 (Zentaro)
 *
 * [Strike]  맹렬한 스트라이크 (0.9s) — 근접 약타. 10회 충전 시 질주 + 관통 투사체
 * [Flip]    에너지 미터       (3s)   — 회피(무적). 에너지 100% 시 주변 폭발 + 코어 제어권
 * [Primary] 산산조각          (7.5s) — 1타: 부채꼴 약타 → 2타: 강 베기 2단 콤보
 * [Secondary] 거합 돌진       (16s)  — 딜레이 후 직선 점멸, 경로 전체 타격
 * [Special] 오니의 칼날       (40s)  — 점멸 + 무적 + 반복 약타 → 최종 전체 타격
 *
 * ※ Ready_CoreHit / Use_CoreHit 오버라이드를 위해 PlayerBase에서 virtual 선언 필요
 */
UCLASS()
class OMEGA_STRIKERS_API AZentaro : public APlayerBase
{
	GENERATED_BODY()

public:
	AZentaro();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
	// ================================================
	//  공용 유틸
	// ================================================
private:
	/**
	 * 부채꼴 히트박스 스캔 후 IOSImpactReceiver에 ImpactData 전달
	 * @param HalfAngleDeg  180.f 이상이면 전방위(원형)
	 */

	void PerformSlash(
		FVector Origin,
		FVector ForwardDir,
		float Range,
		float HalfAngleDeg,
		const FOSImpactData& ImpactData);

	// Impact 수치 계산: base + Power * ratio
	float Calc(float Base, float Ratio) const { return Base + Power * Ratio; }

	// ================================================
	//  [Strike] 맹렬한 스트라이크  쿨: 0.9s
	//  — 근접 약타 / 10회 충전 → 질주 + 관통 투사체
	// ================================================
public:
	// TODO: PlayerBase에서 virtual 선언 필요
	//virtual void Ready_CoreHit() override;
	//virtual void Use_CoreHit() override;

private:
	// 현재 충전 횟수 (0 ~ StrikeChargeMax)
	UPROPERTY(VisibleAnywhere, Category="Skill|Strike")
	int32 StrikeCharge = 0;

	UPROPERTY(EditDefaultsOnly, Category="Skill|Strike")
	int32 StrikeChargeMax = 10;

	// 일반 스트라이크 근접 타격
	void DoNormalStrike();

	// 10회 충전 스트라이크: 질주 + 관통 투사체 스폰
	void DoChargeStrike();

	// 관통 투사체 라인트레이스 (스폰 없이 즉발 처리)
	void FirePenetratingSlash();

	// ================================================
	//  [Flip] 에너지 미터  쿨: 3s
	//  — 에너지 0~100%. 미충전: 회피(무적). 100%: 에너지 폭발
	// ================================================
public:
	virtual void Use_Flip() override;

private:
	// 현재 에너지 (0.f ~ 100.f)
	UPROPERTY(VisibleAnywhere, Category="Skill|EnergyMeter")
	float Energy = 0.f;

	// 코어 타격 시 충전량
	UPROPERTY(EditDefaultsOnly, Category="Skill|EnergyMeter")
	float EnergyPerCoreHit = 10.f;

	// 회피 지속 시간 (초)
	UPROPERTY(EditDefaultsOnly, Category="Skill|EnergyMeter")
	float DodgeDuration = 0.5f;

	// 에너지 폭발 범위
	UPROPERTY(EditDefaultsOnly, Category="Skill|EnergyMeter")
	float ExplosionRange = 400.f;

	// 무적(회피) 상태 체크
	bool bIsDodging = false;

	FTimerHandle DodgeTimerHandle;

	// 일반 회피: 잠시 무적
	void DoEnergyDodge();

	// 100% 에너지 폭발: 주변 밀치기 + 코어 제어권 강화
	void DoEnergyExplosion();

	void EndDodge();

	// ================================================
	//  [Primary] 산산조각  쿨: 7.5s
	//  — 1타: 부채꼴 약타 → 0.2초 후 2타: 강 베기
	// ================================================
public:
	virtual void Use_PrimarySkill() override;

private:
	// 1타 부채꼴 범위
	UPROPERTY(EditDefaultsOnly, Category="Skill|Primary")
	float P1_Range = 200.f;

	UPROPERTY(EditDefaultsOnly, Category="Skill|Primary")
	float P1_HalfAngle = 60.f;

	// 2타 강(強) 베기 범위
	UPROPERTY(EditDefaultsOnly, Category="Skill|Primary")
	float P2_Range = 260.f;

	UPROPERTY(EditDefaultsOnly, Category="Skill|Primary")
	float P2_HalfAngle = 30.f;

	void DoShatteredPhase1();
	void DoShatteredPhase2();

	// ================================================
	//  [Secondary] 거합 돌진  쿨: 16s
	//  — 짧은 딜레이 후 직선 점멸, 경로상 전체 타격
	// ================================================
public:
	virtual void Use_SecondarySkill() override;

	// *거합 : 居合(いあわせ) -> Iawase
private:
	// 점멸 거리
	UPROPERTY(EditDefaultsOnly, Category="Skill|Secondary")
	float IawaseDistance = 900.f;

	// 점멸 전 딜레이 (초)
	UPROPERTY(EditDefaultsOnly, Category="Skill|Secondary")
	float IawaseDelay = 0.3f;

	// 경로 판정 캡슐 반경
	UPROPERTY(EditDefaultsOnly, Category="Skill|Secondary")
	float IawaseHitRadius = 80.f;

	FVector IawaseDirection = FVector::ZeroVector;

	void ExecuteIawaseTeleport();

	// ================================================
	//  [Special] 오니의 칼날  쿨: 40s
	//  — 점멸+무적 → 반복 약타(멀티히트) → 최종 전체 타격
	// ================================================
public:
	virtual void Use_SpecialSkill() override;

	// *오니 : 鬼(おに) -> Oni
private:
	// 점멸 거리
	UPROPERTY(EditDefaultsOnly, Category="Skill|Special")
	float OniTeleportRange = 300.f;

	// 반복 약타 횟수
	UPROPERTY(EditDefaultsOnly, Category="Skill|Special")
	int32 OniMultiHitCount = 8;

	// 반복 약타 간격 (초)
	UPROPERTY(EditDefaultsOnly, Category="Skill|Special")
	float OniMultiHitInterval = 0.15f;

	// 최종 타격 범위
	UPROPERTY(EditDefaultsOnly, Category="Skill|Special")
	float OniFinalRange = 400.f;

	int32 OniCurrentHit = 0;
	FTimerHandle OniMultiHitTimerHandle;

	void OniMultiHitTick();
	void OniFinalBurst();
};
