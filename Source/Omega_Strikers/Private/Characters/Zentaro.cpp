// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/Zentaro.h"

#include "Engine/OverlapResult.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Omega_Strikers/Omega_Strikers.h"

AZentaro::AZentaro()
{
	PrimaryActorTick.bCanEverTick = true;

	// TODO: 데이터 셋 작업 되면 그걸로 사용
	// 기본 스탯
	MaxHP = 1000.f;
	Power = 55.f;
	Speed = 500.f;
	
	// 쿨다운
	
}

void AZentaro::BeginPlay()
{
	Super::BeginPlay();
	Energy = 0.f;
	StrikeCharge = 0;
}

void AZentaro::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// ================================================================
//  공용 유틸 — PerformSlash
//  부채꼴(HalfAngleDeg < 180) 또는 원형(HalfAngleDeg >= 180) 히트박스
// ================================================================

void AZentaro::PerformSlash(FVector Origin, FVector ForwardDir, float Range, float HalfAngleDeg,
	const FOSImpactData& ImpactData)
{
	TArray<FOverlapResult> overlaps;
	FCollisionQueryParams params;
	params.AddIgnoredActor(this);
	
	GetWorld()->OverlapMultiByChannel(overlaps, Origin, FQuat::Identity, 
		ECC_Pawn, FCollisionShape::MakeSphere(Range), params);
	
#if WITH_EDITOR
	DrawDebugSphere(GetWorld(), Origin, Range, 16, FColor::Red, false, 0.4f);
#endif
	
	ForwardDir.Z = 0.f;
	ForwardDir.Normalize();
	
	for (const FOverlapResult& overlap : overlaps)
	{
		AActor* target = overlap.GetActor();
		if (!target || !target->Implements<UOSImpactReceiver>()) continue;
		
		// 부채꼴 각도 필터 (180도 이상이면 전방위로 통과)
		if (HalfAngleDeg < 180.0f)
		{
			FVector toTarget = (target->GetActorLocation() - Origin).GetSafeNormal();
			toTarget.Z = 0.f;
			float angle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ForwardDir, toTarget)));
			if (angle > HalfAngleDeg) continue;
		}
		
		// 타격 방향을 실제 위치 기반으로 덮어쓰기
		FVector hitDir = (target->GetActorLocation() - Origin).GetSafeNormal();
		FOSImpactData finalData = ImpactData;
		finalData.Direction = FVector2D(hitDir.X, hitDir.Y);
		
		Execute_ReceiveImpact(target, finalData, this);
	}
}

// ================================================================
//  [Strike] 맹렬한 스트라이크  쿨: 0.9s
// ================================================================

void AZentaro::Ready_CoreHit()
{
	// TODO: 차징 모션 등 필요 시 여기에 추가
}

void AZentaro::Use_CoreHit()
{
	if (StrikeCharge >= StrikeChargeMax)
	{
		DoChargedStrike();
		StrikeCharge = 0;
	}
	else
	{
		DoNormalStrike();
		StrikeCharge++;
	}
	
	LOG_GT(TEXT("스트라이크 - 충전: %d/%d"), StrikeCharge, StrikeChargeMax);
}

void AZentaro::DoNormalStrike()
{
	// 전방 근접 약타: 코어 밀치기 1230 + Power * 1.25 / 플레이어 125 + Power * 0.62
	FOSImpactData data;
	data.PlayerDamage = Calc(125.f, 0.62f);
	data.PlayerKnockbackPower = Calc(125.f, 0.62f);
	data.CoreKnockbackPower = Calc(1230.f, 1.25f);
	
	PerformSlash(GetActorLocation(), GetActorForwardVector(), 180.f, 50.f, data);
}

void AZentaro::DoChargedStrike()
{
	// 앞으로 질주
	FVector dashDir = GetActorForwardVector();
	dashDir.Z = 0.f;
	dashDir.Normalize();
	LaunchCharacter(dashDir * 1800.f, true, false);
	
	// 0.1초 후 관통 투사체 (라인트레이스)
	FTimerHandle timer;
	GetWorldTimerManager().SetTimer(timer, [this]()
	{
		FirePenetratingSlash();
	}, 0.1f, false);
	
	LOG_GT(TEXT("맹렬한 스트라이크 시전"));
}

void AZentaro::FirePenetratingSlash()
{
	// 관통: 전방 직선 라인트레이스, 최초 적중한 1명 타격
	FVector start = GetActorLocation();
	FVector end = start + GetActorForwardVector() * 1200.f;
	
	FHitResult hit;
	FCollisionQueryParams params;
	params.AddIgnoredActor(this);
	
	bool bHit = GetWorld()->LineTraceSingleByChannel(hit, start, end, ECC_Pawn, params);
	
#if WITH_EDITOR
	DrawDebugLine(GetWorld(), start, end, FColor::Magenta, false, 0.5f, 0, 2.f);
#endif
	
	if (bHit && hit.GetActor() && hit.GetActor()->Implements<UOSImpactReceiver>())
	{
		FOSImpactData data;
		data.PlayerDamage = Calc(125.f, 0.62f);
		data.PlayerKnockbackPower = Calc(125.f, 0.62f);
		data.CoreKnockbackPower = Calc(1230.f, 1.25f);
		
		FVector dir = (hit.GetActor()->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		data.Direction = FVector2D(dir.X, dir.Y);
		
		Execute_ReceiveImpact(hit.GetActor(), data, this);
	}
}

// ================================================================
//  [Flip] 에너지 미터  쿨: 3s
// ================================================================

void AZentaro::Use_Flip()
{
	if (FMath::IsNearlyEqual(Energy, 100.f, 1.f))
	{
		DoEnergyExplosion();
	}
	else
	{
		DoEnergyDodge();
	}
}

void AZentaro::DoEnergyDodge()
{
	// 회피: 무적 상태 활성화 (피해/밀치기 면역)
	bIsDodging = true;
	Energy = FMath::Max(0.f, Energy - 30.f); // 에너지 소비
	
	// 이동 입력 방향으로 짧은 대시
	FVector dodgeDir = GetCharacterMovement()->GetLastInputVector();
	if (dodgeDir.IsNearlyZero()) dodgeDir = GetActorForwardVector();
	dodgeDir.Z = 0.f;
	dodgeDir.Normalize();
	LaunchCharacter(dodgeDir * 900.f, true, false);
	
	// DodgeDuration 후 무적 해제
	GetWorldTimerManager().SetTimer(DodgeTimerHandle, this, &AZentaro::EndDodge, DodgeDuration, false);
	
	LOG_GT(TEXT("에너지 회피 (에너지: %.0f%%)"), Energy);
}

void AZentaro::DoEnergyExplosion()
{
	// 에너지 폭발: 주변 적 밀치기 + 코어 제어권
	FOSImpactData data;
	data.PlayerDamage = Calc(200.f, 1.f);
	data.PlayerKnockbackPower = Calc(200.f, 1.f);
	data.CoreKnockbackPower = 0.f; // 코어 타격은 별도 강화 버프로 처리
	
	// 360도 전방위
	PerformSlash(GetActorLocation(), GetActorForwardVector(), ExplosionRange, 180.f, data);
	
	Energy = 0.f; // 에너지 전부 소진
	
	// TODO: 다음 코어 타격 강화 버프 (별도 bool 플래그로 관리 가능)
	
	LOG_GT(TEXT("에너지 폭발!"));
}

void AZentaro::EndDodge()
{
	bIsDodging = false;
}

// ================================================================
//  [Primary] 산산조각  쿨: 7.5s
//  1타(부채꼴 약타) → 0.2초 후 2타(강 베기)
// ================================================================

void AZentaro::Use_PrimarySkill()
{
	DoShatteredPhase1();
}

void AZentaro::DoShatteredPhase1()
{
	// 1타: 작은 부채꼴 약타
	// 코어 100 + Power * 0.1 / 플레이어 6 + Power * 0.2
	FOSImpactData data;
	data.PlayerDamage = Calc(6.f, 0.02f);
	data.PlayerKnockbackPower = Calc(6.f, 0.02f);
	data.CoreKnockbackPower = Calc(100.f, 0.1f);
	
	PerformSlash(GetActorLocation(), GetActorForwardVector(), P1_Range, P1_HalfAngle, data);
	
	// 0.2초 후 2타 시전
	FTimerHandle timer;
	GetWorldTimerManager().SetTimer(timer, [this]()
	{
		DoShatteredPhase2();
	}, 0.2f, false);
	
	LOG_GT(TEXT("산산조각 1타"));
}

void AZentaro::DoShatteredPhase2()
{
	// 2타: 좁은 부채꼴 강 베기 (중앙 집중)
	// 수치는 1타와 동일하되 범위가 좁고 강함 (원작 기준)
	FOSImpactData data;
	data.PlayerDamage = Calc(6.f, 0.02f);
	data.PlayerKnockbackPower = Calc(6.f, 0.02f);
	data.CoreKnockbackPower = Calc(100.f, 0.1f);
	
	PerformSlash(GetActorLocation(), GetActorForwardVector(), P2_Range, P2_HalfAngle, data);
	
	LOG_GT(TEXT("산산조각 2타"));
}

// ================================================================
//  [Secondary] 거합 돌진  쿨: 16s
//  딜레이 후 직선 점멸, 경로상 모든 적 타격
// ================================================================

void AZentaro::Use_SecondarySkill()
{
	IawaseDirection = GetCharacterMovement()->GetLastInputVector();
	if (IawaseDirection.IsNearlyZero()) IawaseDirection = GetActorForwardVector();
	IawaseDirection.Z = 0.f;
	IawaseDirection.Normalize();
	
	// 딜레이 후 점멸
	FTimerHandle timer;
	GetWorldTimerManager().SetTimer(timer, [this]()
	{
		ExecuteIawaseTeleport();
	}, IawaseDelay, false);
	
	LOG_GT(TEXT("거합 돌진 준비"));
}

void AZentaro::ExecuteIawaseTeleport()
{
	FVector start = GetActorLocation();
	FVector end = start + IawaseDirection * IawaseDistance;
	
	// 경로상의 적 스윕 히트
	TArray<FHitResult> hits;
	FCollisionQueryParams params;
	params.AddIgnoredActor(this);
	
	GetWorld()->SweepMultiByChannel(hits, start, end, FQuat::Identity, 
		ECC_Pawn, FCollisionShape::MakeSphere(IawaseHitRadius), params);
	
	FOSImpactData data;
	data.PlayerDamage = Calc(170.f, 0.85f);
	data.PlayerKnockbackPower = Calc(170.f, 0.85f);
	data.CoreKnockbackPower = Calc(1320.f, 1.7f);
	
	for (const FHitResult& hit : hits)
	{
		AActor* target = hit.GetActor();
		if (!target || !target->Implements<UOSImpactReceiver>()) continue;
		
		FVector dir = (target->GetActorLocation() - start).GetSafeNormal();
		data.Direction = FVector2D(dir.X, dir.Y);
		Execute_ReceiveImpact(target, data, this);
	}
	
	// 점멸: 도착 위치로 순간이동
	SetActorLocation(end, true);
	SetActorRotation(IawaseDirection.Rotation());
	
#if WITH_EDITOR
	DrawDebugLine(GetWorld(), start, end, FColor::Purple, false, 0.5f, 0, 3.f);
#endif

	LOG_GT(TEXT("거합 돌진 시전! 피격: %d명"), hits.Num());
}

// ================================================================
//  [Special] 오니의 칼날  쿨: 40s
//  점멸 + 무적 → 반복 약타(OniMultiHitCount회) → 최종 전체 타격
// ================================================================

void AZentaro::Use_SpecialSkill()
{
	// 이동 방향으로 짧게 점멸
	FVector teleDir = GetCharacterMovement()->GetLastInputVector();
	if (teleDir.IsNearlyZero()) teleDir = GetActorForwardVector();
	teleDir.Z = 0.f;
	teleDir.Normalize();
	
	FVector teleportDest = GetActorLocation() + teleDir * OniTeleportRange;
	SetActorLocation(teleportDest, true);
	
	// 무적 활성화
	bIsDodging = true;
	OniCurrentHit = 0;
	
	// 반복 약타 시작
	GetWorldTimerManager().SetTimer(
		OniMultiHitTimerHandle, this, &AZentaro::OniMultiHitTick, OniMultiHitInterval, true);
	
	LOG_GT(TEXT("오니의 칼날 시전!"));
}

void AZentaro::OniMultiHitTick()
{
	OniCurrentHit++;
	
	// 주변 적에게 반복 약타: 코어 20 + Power * 0.1 / 플레이어 2
	FOSImpactData data;
	data.PlayerDamage = 2.f;
	data.PlayerKnockbackPower = 2.f;
	data.CoreKnockbackPower = Calc(20.f, 0.1f);
	
	PerformSlash(GetActorLocation(), GetActorForwardVector(), OniFinalRange * 0.6f, 180.f, data);
	
	if (OniCurrentHit >= OniMultiHitCount)
	{
		GetWorldTimerManager().ClearTimer(OniMultiHitTimerHandle);
		OniFinalBurst();
	}
}

void AZentaro::OniFinalBurst()
{
	// 최종 타격: 근처 전체 적
	FOSImpactData data;
	data.PlayerDamage = Calc(6.f, 0.02f);
	data.PlayerKnockbackPower = Calc(6.f, 0.02f);
	data.CoreKnockbackPower = Calc(20.f, 0.1f);
	
	PerformSlash(GetActorLocation(), GetActorForwardVector(), OniFinalRange, 180.f, data);
	
	// 무적 해제
	bIsDodging = false;
	
	LOG_GT(TEXT("오니의 칼날 최종 타격!"));
}

