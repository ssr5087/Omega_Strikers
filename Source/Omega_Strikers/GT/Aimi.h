// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerBase.h"
#include "Omega_Strikers/SSR/CharacterStat.h"

#include "Aimi.generated.h"

class AAimiFirewallSentry;
class AAimiGlitchOrb;
/**
 * 아이미 (Ai.Mi) — "The Girl Who Glitched"
 *
 * APlayerBase 직접 상속 (젠타로와 동일 구조)
 *
 * ───────────────────────────────────────────────
 *  [Strike]    스트라이크        (0.9s)  근접 약타
 *  [Flip]      에너지 미터       (3s)    회피 / 100% → 에너지 폭발
 *  [Primary]   글리치.팝         (8s)    성장형 오브 → 재시전 폭발 → 방사형 Push
 *  [Secondary] 사이버 스와이프   (14s)   점멸 + 꼬리 강타 (부채꼴)
 *  [Special]   방화벽 파수꾼     (30s)   터렛 설치 → 기술 억제 방화벽 연사 (첫 적중만)
 * ───────────────────────────────────────────────
 *
 * ★ 포트폴리오 핵심:
 *   - 글리치.팝: 폭발 Push Vector = normalize(Target - OrbCenter) × force
 *   - 재시전 시스템: 1클릭 발사 / 2클릭 폭발 (스킬 상태 머신)
 *   - 점멸 + 도착지 부채꼴 히트박스
 */
UCLASS()
class OMEGA_STRIKERS_API AAimi : public APlayerBase
{
	GENERATED_BODY()

public:
	AAimi();

	// 대시 체크
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bIsDashing = false;

	// 글리치팝 오브 날리는 중인지 체크
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bIsAimingOrb = false;

	// Sentry 설치중인지 체크
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bIsPlacingSentry = false;

	// 서지 중인지 체크
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bIsSurging = false;
protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

// public:
// 	// DataTable
// 	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat")
// 	UDataTable* CharacterStatTable;
// 	
// 	// 캐릭터 이름 (Asher 고정이면 기본값 줘도 됨)
// 	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stat")
// 	FName CharacterName = "Aimi";
//
// 	// 레벨 (나중 대비)
// 	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stat")
// 	int32 Level = 1;
//
// 	// 현재 스탯 저장
// 	FCharacterStat CurrentStat;
//
// 	// 실제 적용되는 값
// 	float MaxHP;
// 	float Power;
// 	float MoveSpeed;
// 	float CooldownReduction;
//
// 	// 함수
// 	FCharacterStat* GetStatByLevel(int32 InLevel);
// 	void ApplyStat(const FCharacterStat& Stat);
// 	void LevelUp();
// 	
public:

	// ════════════════════════════════════════════
	//  PlayerBase virtual 구현
	// ════════════════════════════════════════════
	virtual void Ready_CoreHit() override;
	virtual void Use_CoreHit() override;
	virtual void Ready_PrimarySkill() override;
	virtual void Use_PrimarySkill() override;
	virtual void Ready_SecondarySkill() override;
	virtual void Use_SecondarySkill() override;
	virtual void Ready_SpecialSkill() override;		
	virtual void Use_SpecialSkill() override;
	virtual void Ready_Flip() override;
	virtual void Use_Flip() override;

protected:
	
	
	// Ready 중 매 프레임 갱신되는 캐싱된 에이밍 방향
	FVector CachedAimDirection = FVector::ForwardVector;
	
	// ════════════════════════════════════════════
	//  쿨다운
	// ════════════════════════════════════════════
	UPROPERTY(EditDefaultsOnly, Category = "Cooldown")
	float CoreHitCool_Max = 0.9f;

 	UPROPERTY(EditDefaultsOnly, Category = "Cooldown")
	float PrimaryCool_Max = 8.f;

 	UPROPERTY(EditDefaultsOnly, Category = "Cooldown")
	float SecondaryCool_Max = 14.f;

	UPROPERTY(EditDefaultsOnly, Category = "Cooldown")
	float CD_Special_Max = 30.f;
	
	// CoolDownRate 스탯 반영한 실제 쿨다운
	float GetAdjustedCD(float Base) const;

	void TickCooldowns(float DeltaTime);

	// ════════════════════════════════════════════
	//  [Strike] 스트라이크 — 근접 약타
	// ════════════════════════════════════════════
	UPROPERTY(EditDefaultsOnly, Category = "Strike")
	float StrikeRange = 180.f;

	UPROPERTY(EditDefaultsOnly, Category = "Strike")	
	float StrikeHalfAngle = 55.f;

	UPROPERTY(EditDefaultsOnly, Category = "Strike")
	float StrikeCoreKnockback = 1293.f;

	UPROPERTY(EditDefaultsOnly, Category = "Strike")
	float StrikePlayerKnockback = 156.f;

	UPROPERTY(EditDefaultsOnly, Category = "Strike")
	float StrikeDamage = 156.f;

	// 에너지 미터 충전량 ( 0 ~ 100 )
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Energy")
	float Energy = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "Energy")
	float EnergyPerStrike = 8.f;

private:
	void DoStrike();

protected:
	// ════════════════════════════════════════════
	//  [Primary] 글리치.팝 — 성장형 오브 + 재시전 폭발
	//
	//  ★ 상태 머신:
	//     Ready → 발사(SpawnOrb) → 오브 비행 중 → 재시전(Detonate)
	//     Use_PrimarySkill에서 오브 유무로 분기
	// ════════════════════════════════════════════
	
	// GlitchOrb BP 클래스 (에디터에서 지정)
	UPROPERTY(EditDefaultsOnly, Category = "Primary|GlitchPop")
	TSubclassOf<AAimiGlitchOrb> GlitchOrbClass;

	// 현재 활성 오브 참조
	UPROPERTY()
	TObjectPtr<AAimiGlitchOrb> ActiveOrb;

private:
	// 오브 발사
	void FireGlitchOrb();

	// 오브 재시전 (폭발)
	void RecastGlitchOrb();

protected:
	// ════════════════════════════════════════════
	//  [Secondary] 사이버 스와이프 — 점멸 + 꼬리 강타
	//
	//  ★ 구현: SetActorLocation 점멸 → 딜레이 → PerformSlash
	// ════════════════════════════════════════════
	// 점멸 거리
	UPROPERTY(EditDefaultsOnly, Category = "Secondary|CyberSwipe")
	float BlinkDistance = 500.f;

	// 도착지 타격 범위
	UPROPERTY(EditDefaultsOnly, Category = "Secondary|CyberSwipe")
	float SwipeRange = 200.f;

	// 도착지 타격 반각
	UPROPERTY(EditDefaultsOnly, Category = "Secondary|CyberSwipe")
	float SwipeHalfAngle = 120.f;

	// 코어 밀치기
	UPROPERTY(EditDefaultsOnly, Category = "Secondary|CyberSwipe")
	float SwipeCoreKnockback = 1411.f;

	// 플레이어 밀치기
	UPROPERTY(EditDefaultsOnly, Category = "Secondary|CyberSwipe")
	float SwipePlayerKnockback = 262.f;

	// 피해
	UPROPERTY(EditDefaultsOnly, Category = "Secondary|CyberSwipe")
	float SwipeDamage = 262.f;

private:
	// 점멸 목표 위치 ( 마우스 / 입력 방향 기반 )
	FVector CachedBlinkTarget;

	void DoCyberSwipe();

	// 점멸 후 딜레이 콜백 - 꼬리 강타 실행
	void OnCyberSwipeArrived();

protected:
	// ════════════════════════════════════════════
	//  [Special] 파이어월 센트리 — 터렛 설치
	// ════════════════════════════════════════════
	// 센트리 BP 클래스 (에디터에서 지정)
	UPROPERTY(EditDefaultsOnly, Category = "Secondary|Sentry")
	TSubclassOf<AAimiFirewallSentry> SentryClass;

	// 최대 동시 설치 수
	UPROPERTY(EditDefaultsOnly, Category = "Secondary|Sentry")
	int32 MaxSentries = 1;

	// 현재 설치된 센트리 목록
	UPROPERTY()
	TArray<TObjectPtr<AAimiFirewallSentry>> ActiveSentries;

private:
	void PlaceSentry();

protected:
	// ════════════════════════════════════════════
    //  [Flip] 에너지 미터 — 회피 / 에너지 폭발
    //
    //  ★ Energy < 100: 무적 대시 (회피)
    //  ★ Energy == 100: 에너지 폭발 (360° 밀어내기 + 코어 강타 버프)
    // ════════════════════════════════════════════
	// 회피 대시 거리
	UPROPERTY(EditDefaultsOnly, Category = "Flip|Energy")
	float DodgeDashForce = 1200.f;

	// 에너지 폭발 범위
	UPROPERTY(EditDefaultsOnly, Category = "Flip|Energy")
	float EnergyBurstRange = 250.f;

	// 에너지 폭발 밀치기
	UPROPERTY(EditDefaultsOnly, Category = "Flip|Energy")
	float EnergyBurstKnockback = 250.f;

	// 에너지 폭발 피해
	UPROPERTY(EditDefaultsOnly, Category = "Flip|Energy")
	float EnergyBurstDamage = 250.f;

private:
	void DoDodge();
	void DoEnergyBurst();

	// ════════════════════════════════════════════
	//  유틸리티
	// ════════════════════════════════════════════
	/**
	 * 부채꼴 히트박스 판정
	 * 구형 오버랩 → 전방 각도 필터 → IOSImpactReceiver에 ImpactData 전달
	 * @return 하나라도 맞았으면 true
	 */
	bool PerformSlash(const FVector& Origin, const FVector& ForwardDir,
					  float Range, float HalfAngleDeg,
					  const FOSImpactData& InData);
	
	// ════════════════════════════════════════════
	//  커서 에이밍 + 조준 UI
	// ════════════════════════════════════════════
	
	// 마우스 커서 -> 바닥 히트 -> 월드 좌표
	FVector GetCursorWorldPosition() const;
	
	// 캐릭터 -> 커서 방향 (Z = 0)
	FVector GetAimDirection() const;
	
	// Tick에서 호출 - 플래그에 따라 조준선 그리기
	void DrawAimIndicator();
	void UpdateAimIndicator();
	
	void DrawGlitchPopAim();
	void DrawCyberSwipeAim();
	void DrawSentryAim();
	
	void DrawGroundCircle(const FVector& Center, float Radius, const FColor& Color, int32 Segments = 32, float Thickness = 2.f) const;
	
	void DrawArcOnGround(const FVector& Center, const FVector& Forward, float Radius, float HalfAngleDeg, const FColor& Color, int32 Segments = 16, float Thickness = 2.f) const;

	// ════════════════════════════════════════════
	//  Niagara
	// ════════════════════════════════════════════

	// 에임 인디케이터
	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	TObjectPtr<class UNiagaraSystem> NS_AimIndicator;

	UPROPERTY()
	TObjectPtr<class UNiagaraComponent> AimIndicatorComp;

	UPROPERTY()
	TObjectPtr<class UNiagaraComponent> DecalIndicatorComp;
};
