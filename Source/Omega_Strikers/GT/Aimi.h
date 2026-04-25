// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerBase.h"
#include "Aimi.generated.h"

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

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// ════════════════════════════════════════════
	//  PlayerBase virtual 구현
	// ════════════════════════════════════════════
	
};
