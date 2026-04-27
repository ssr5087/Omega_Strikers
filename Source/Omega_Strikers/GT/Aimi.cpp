// Fill out your copyright notice in the Description page of Project Settings.


#include "Aimi.h"

#include "AimiFirewallSentry.h"
#include "AimiGlitchOrb.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Omega_Strikers/Omega_Strikers.h"
#include "Omega_Strikers/SM/HPComponent.h"
#include "Omega_Strikers/SSR/CharacterSkill.h"

AAimi::AAimi()
{
	PrimaryActorTick.bCanEverTick = true;

	// TODO: PlayerBase 기본 스탯 확인 후 수정
	// Power, Speed 등은 PlayerBase에서 관리
}

void AAimi::BeginPlay()
{
	Super::BeginPlay();

	CD_Strike = 0.f;
	CD_Primary = 0.f;
	CD_Secondary = 0.f;
	CD_Special = 0.f;
	CD_Flip = 0.f;
	Energy = 0.f;
	ActiveOrb = nullptr;
	
	// 데이터 셋
	FCharacterStat* Stat = GetStatByLevel(Level);
	
	if (Stat)
	{
		ApplyStat(*Stat);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Stat Load Failed"));
	}
}

void AAimi::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	TickCooldowns(DeltaTime);

	// 활성 오브가 파괴됐으면 참조 정리
	if (ActiveOrb && (ActiveOrb->IsPendingKillPending() || ActiveOrb->HasDetonated()))
	{
		ActiveOrb = nullptr;
	}

	// 활성 센트리 중 파괴된 것 정리
	ActiveSentries.RemoveAll([](const TObjectPtr<AAimiFirewallSentry>& sentry)
	{
		return !IsValid(sentry);
	});
}

FCharacterStat* AAimi::GetStatByLevel(int32 InLevel)
{
	if (!CharacterStatTable)
	{
		UE_LOG(LogTemp, Error, TEXT("CharacterStatTable is NULL"));
		return nullptr;
	}
	
	FName RowName = FName(*FString::Printf(TEXT("%s_%d"), *CharacterName.ToString(), InLevel));
	
	UE_LOG(LogTemp, Warning, TEXT("Trying Row: %s"), *RowName.ToString());
	
	return CharacterStatTable->FindRow<FCharacterStat>(RowName, TEXT(""));
}

void AAimi::ApplyStat(const FCharacterStat& Stat)
{
	CurrentStat = Stat;
	
	// PlayerBase 변수 덮어쓰기
	MaxHP = Stat.MaxHP;
	Power = Stat.Power;
	Speed = Stat.Speed;
	CoolDownRate = Stat.Cooldown;
	
	// 이동속도 적용
	GetCharacterMovement()->MaxWalkSpeed = Speed;
	
	if (HPComp)
	{
		HPComp->UpdateMaxHP(MaxHP);
		HPComp->InitializeHP();
	}
	
	// 테스트 용
	UE_LOG(LogTemp, Warning, TEXT("HP: %.1f / Power: %.1f / Speed: %.1f"),
	MaxHP, Power, Speed);
}

void AAimi::LevelUp()
{
	Level++;
	
	FCharacterStat* Stat = GetStatByLevel(Level);
	
	if (Stat)
	{
		ApplyStat(*Stat);
	}
}


// ════════════════════════════════════════════════════════════
//  쿨다운
// ════════════════════════════════════════════════════════════
void AAimi::TickCooldowns(float DeltaTime)
{
	if (CD_Strike	 > 0.f) CD_Strike	 = FMath::Max(0.f, CD_Strike	 - DeltaTime);
	if (CD_Primary   > 0.f) CD_Primary	 = FMath::Max(0.f, CD_Primary	 - DeltaTime);
	if (CD_Secondary > 0.f) CD_Secondary = FMath::Max(0.f, CD_Secondary - DeltaTime);
	if (CD_Special	 > 0.f) CD_Special	 = FMath::Max(0.f, CD_Special   - DeltaTime);
	if (CD_Flip		 > 0.f) CD_Flip		 = FMath::Max(0.f, CD_Flip		 - DeltaTime);
}

float AAimi::GetAdjustedCD(float Base) const
{
	// CoolDownRate: PlayerBase 스탯. 양수 = 쿨감
	const float Adjusted = Base * (1.f - CoolDownRate);
	return FMath::Max(0.1f, Adjusted);
}

// ════════════════════════════════════════════════════════════
//  [Strike] 스트라이크 — 근접 약타
//
//  에너지 미터 충전: StrikeHit → Energy += EnergyPerStrike
// ════════════════════════════════════════════════════════════
void AAimi::Ready_CoreHit()
{
	// 차징 없이 즉시 사용
	Use_CoreHit();
}

void AAimi::Use_CoreHit()
{
	if (CD_Strike > 0.f) return;

	DoStrike();
	CD_Strike = GetAdjustedCD(CD_Strike_Max);
}

void AAimi::DoStrike()
{
	const FVector origin = GetActorLocation();
	const FVector forward = GetActorForwardVector();
	const FVector2D dir2D = FVector2D(forward.X, forward.Y).GetSafeNormal();

	// PerformSlash: PlayerBase에 정의된 부채꼴 히트박스 유티리티
	FOSImpactData strikeData;
	strikeData.TeamSide = TeamSide;
	strikeData.Direction = dir2D;
	strikeData.CoreKnockbackPower = StrikeCoreKnockback;
	strikeData.PlayerKnockbackPower = StrikePlayerKnockback;
	strikeData.PlayerDamage = StrikeDamage;

	const bool bHitSomething = PerformSlash(origin, forward, StrikeRange, StrikeHalfAngle, strikeData);

	if (bHitSomething)
	{
		Energy = FMath::Min(100.f, Energy + EnergyPerStrike);
		LOG_GT(TEXT("Energy: %.0f"), Energy);
	}

	LOG_GT(TEXT("Range: %.0f, Angle: ±%.0f°"), StrikeRange, StrikeHalfAngle);
}

// ════════════════════════════════════════════════════════════
//  [Primary] 글리치.팝 — 재시전 시스템
// ════════════════════════════════════════════════════════════
void AAimi::Ready_PrimarySkill()
{
	Use_PrimarySkill();
}

void AAimi::Use_PrimarySkill()
{
	if (ActiveOrb && !ActiveOrb->HasDetonated())
	{
		RecastGlitchOrb();
	}
	else
	{
		if (CD_Primary > 0.f) return;
		FireGlitchOrb();
		CD_Primary = GetAdjustedCD(CD_Primary_Max);
	}
}

void AAimi::FireGlitchOrb()
{
	if (!GlitchOrbClass)
	{
		LOG_GT_E(TEXT("GlitchOrbClass not set!"));
		return;
	}

	const FVector spawnLoc = GetActorLocation() + GetActorForwardVector() * 60.f;
	const FVector aimDir = GetActorForwardVector();

	FActorSpawnParameters spawnParams;
	spawnParams.Owner = this;
	spawnParams.Instigator = this;
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ActiveOrb = GetWorld()->SpawnActor<AAimiGlitchOrb>(GlitchOrbClass, spawnLoc, aimDir.Rotation(), spawnParams);

	if (ActiveOrb)
	{
		ActiveOrb->Launch(this, aimDir);
		LOG_GT(TEXT("Glitch.Pop - Orb launched"));
	}
}

void AAimi::RecastGlitchOrb()
{
	if (!ActiveOrb || ActiveOrb->HasDetonated()) return;
	ActiveOrb->Detonate();
	LOG_GT(TEXT("Glitch.Pop - RECAST! Orb Detonated"));
}

// ════════════════════════════════════════════════════════════
//  [Secondary] 사이버 스와이프 — 점멸 + 꼬리 강타
// ════════════════════════════════════════════════════════════
void AAimi::Ready_SecondarySkill()
{
	if (CD_Secondary > 0.f) return;
	Use_SecondarySkill();
}

void AAimi::Use_SecondarySkill()
{
	if (CD_Secondary > 0.f) return;
	DoCyberSwipe();
	CD_Secondary = GetAdjustedCD(CD_Secondary_Max);
}

void AAimi::DoCyberSwipe()
{
	FVector blinkDir = GetActorForwardVector();
	blinkDir.Z = 0.f;
	blinkDir.Normalize();

	const FVector inputDir = GetCharacterMovement()->GetLastInputVector();
	if (!inputDir.IsNearlyZero())
	{
		blinkDir = inputDir.GetSafeNormal();
		blinkDir.Z = 0.f;
	}

	const FVector start = GetActorLocation();
	const FVector target = start + blinkDir * BlinkDistance;

	FHitResult wallHit;
	FCollisionQueryParams params;
	// TODO: 아군 플레이어도 충돌 무시 처리해야함
	params.AddIgnoredActor(this);
	bool bBlocked = GetWorld()->LineTraceSingleByChannel(wallHit, start, target, ECC_WorldDynamic, params);

	const FVector finalPos = bBlocked ? wallHit.ImpactPoint - blinkDir * 30.f : target;

	SetActorLocation(finalPos);
	SetActorRotation(blinkDir.Rotation());
	CachedBlinkTarget = finalPos;

	LOG_GT(TEXT("Cyber Swipe - Blinked"));

	FTimerHandle timerHandle;
	GetWorldTimerManager().SetTimer(timerHandle, this, &AAimi::OnCyberSwipeArrived, 0.05f, false);
}

void AAimi::OnCyberSwipeArrived()
{
	const FVector origin = GetActorLocation();
	const FVector forward = GetActorForwardVector();
	const FVector2D dir2D = FVector2D(forward.X, forward.Y).GetSafeNormal();

	FOSImpactData data;
	data.TeamSide = TeamSide;
	data.Direction = dir2D;
	data.CoreKnockbackPower = SwipeCoreKnockback;
	data.PlayerKnockbackPower = SwipePlayerKnockback;
	data.PlayerDamage = SwipeDamage;

	PerformSlash(origin, forward, SwipeRange, SwipeHalfAngle, data);
	
	LOG_GT(TEXT("Cyber Swipe - Tail slash!"));
}

// ════════════════════════════════════════════════════════════
//  [Special] 방화벽 파수꾼 — 터렛 설치
// ════════════════════════════════════════════════════════════

void AAimi::Ready_SpecialSkill()
{
	if (CD_Special > 0.f) return;
	Use_SpecialSkill();
}

void AAimi::Use_SpecialSkill()
{
	if (CD_Special > 0.f) return;
	PlaceSentry();
	CD_Special = GetAdjustedCD(CD_Special_Max);
}

void AAimi::PlaceSentry()
{
	if (!SentryClass)
	{
		LOG_GT_E(TEXT("SentryClass not set!"));
		return;
	}

	while (ActiveSentries.Num() >= MaxSentries)
	{
		AAimiFirewallSentry* oldest = ActiveSentries[0];
		if (IsValid(oldest)) oldest->Destroy();
		ActiveSentries.RemoveAt(0);
	}

	const FVector forward = GetActorForwardVector();
	const FVector spawnLoc = GetActorLocation() + forward * 80.f;

	FActorSpawnParameters spawnParams;
	spawnParams.Owner = this;
	spawnParams.Instigator = this;
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AAimiFirewallSentry* sentry = GetWorld()->SpawnActor<AAimiFirewallSentry>(SentryClass, spawnLoc, forward.Rotation(), spawnParams);

	if(sentry)
	{
		sentry->Initialize(this, forward);
		ActiveSentries.Add(sentry);
		LOG_GT(TEXT("Firewall Sentry placed"));
	}
}

// ════════════════════════════════════════════════════════════
//  [Flip] 에너지 미터
// ════════════════════════════════════════════════════════════
void AAimi::Ready_Flip()
{
	if (CD_Flip > 0.f) return;
	Use_Flip();
}

void AAimi::Use_Flip()
{
	if (CD_Flip > 0.f) return;
	Energy >= 100.f ? DoEnergyBurst() : DoDodge();
	CD_Flip = GetAdjustedCD(CD_Flip_Max);
}

void AAimi::DoDodge()
{
	FVector dashDir = GetCharacterMovement()->GetLastInputVector();
	if (dashDir.IsNearlyZero()) dashDir = GetActorForwardVector();
	dashDir.Z = 0.f;
	dashDir.Normalize();

	LaunchCharacter(dashDir * DodgeDashForce, true, false);
	LOG_GT(TEXT("Dodge done"));
}

void AAimi::DoEnergyBurst()
{
	Energy = 0.f;

	const FVector origin = GetActorLocation();
	const FVector forward = GetActorForwardVector();
	const FVector2D dir2D = FVector2D(forward.X, forward.Y).GetSafeNormal();

	FOSImpactData data;
	data.TeamSide = TeamSide;
	data.Direction = dir2D;
	data.CoreKnockbackPower = 0.f;
	data.PlayerKnockbackPower = EnergyBurstKnockback;
	data.PlayerDamage = EnergyBurstDamage;

	PerformSlash(origin, forward, EnergyBurstRange, 180.f, data);
	LOG_GT(TEXT("ENERGY BURST! 360°"));
}

// ════════════════════════════════════════════════════════════
//  PerformSlash — 부채꼴 히트박스 (젠타로와 동일 구조)
//
//  1. 구형 OverlapMulti로 Range 내 Actor 수집
//  2. 내적(DotProduct)으로 전방 HalfAngle 이내 필터
//  3. IOSImpactReceiver 구현체에 FOSImpactData 전달
// ════════════════════════════════════════════════════════════

bool AAimi::PerformSlash(const FVector& Origin, const FVector& ForwardDir,
                         float Range, float HalfAngleDeg,
                         const FOSImpactData& InData)
{
	FVector Forward = ForwardDir;
	Forward.Z = 0.f;
	Forward.Normalize();
 
	TArray<FOverlapResult> Overlaps;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(Range);
	FCollisionQueryParams Params;
	// TODO: 아군 플레이어도 충돌 무시 처리해야함
	Params.AddIgnoredActor(this);
 
	GetWorld()->OverlapMultiByChannel(
		Overlaps, Origin, FQuat::Identity,
		ECC_Pawn, Sphere, Params);
 
#if WITH_EDITOR
	DrawDebugSphere(GetWorld(), Origin, Range, 16, FColor::Orange, false, 0.5f);
#endif
 
	const float CosThreshold = FMath::Cos(FMath::DegreesToRadians(HalfAngleDeg));
	bool bHitAny = false;
 
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Target = Overlap.GetActor();
		if (!Target || !Target->Implements<UOSImpactReceiver>()) continue;
 
		// 각도 필터 (180° = 전방위 → 스킵)
		if (HalfAngleDeg < 180.f)
		{
			FVector ToTarget = (Target->GetActorLocation() - Origin);
			ToTarget.Z = 0.f;
			ToTarget.Normalize();
			if (FVector::DotProduct(Forward, ToTarget) < CosThreshold) continue;
		}
 
		// 대상별 밀어내기 방향 (Origin → Target, 2D)
		FVector PushDir3D = (Target->GetActorLocation() - Origin);
		PushDir3D.Z = 0.f;
		PushDir3D.Normalize();
		const FVector2D PushDir2D = FVector2D(PushDir3D.X, PushDir3D.Y).GetSafeNormal();
 
		// FOSImpactData — 방향만 대상별로 재설정
		FOSImpactData Data = InData;
		Data.Direction = PushDir2D;
 
		IOSImpactReceiver::Execute_ReceiveImpact(Target, Data, this);
		bHitAny = true;
 
		LOG_GT(TEXT("[AiMi] Slash hit %s — Dir:(%.2f,%.2f) CoreKB:%.0f PlayerKB:%.0f Dmg:%.0f"),
			*Target->GetName(), PushDir2D.X, PushDir2D.Y,
			Data.CoreKnockbackPower, Data.PlayerKnockbackPower, Data.PlayerDamage);
	}
 
	return bHitAny;
}
