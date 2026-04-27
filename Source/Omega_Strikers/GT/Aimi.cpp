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
	
	LOG_GT(TEXT("Tick — bAimingPrimary: %d"), bAimingPrimary);
	DrawAimIndicator();
}

// ════════════════════════════════════════════════════════════
//  ClearAllAiming
// ════════════════════════════════════════════════════════════
void AAimi::ClearAllAiming()
{
	bAimingPrimary   = false;
	bAimingSecondary = false;
	bAimingSpecial   = false;
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
	LOG_GT(TEXT("Ready_PrimarySkill called! bAimingPrimary will be set true"));
	
	if (ActiveOrb && !ActiveOrb->HasDetonated())
	{
		bAimingPrimary = true;  // 재시전 대기 UI
		return;
	}
	if (CD_Primary > 0.f) return;

	ClearAllAiming();
	bAimingPrimary = true;  // ← 이게 켜지면 Tick에서 궤적선 그림
}

void AAimi::Use_PrimarySkill()
{
	LOG_GT(TEXT("Use_PrimarySkill called! — bAimingPrimary was %d"), bAimingPrimary);
	
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

// ════════════════════════════════════════════════════════════
//  커서 에이밍
// ════════════════════════════════════════════════════════════
FVector AAimi::GetCursorWorldPosition() const
{
	APlayerController* pc = Cast<APlayerController>(GetController());
	if (!pc) return GetActorLocation() + GetActorForwardVector() * 300.f;
	
	FHitResult hit;
	if (pc->GetHitResultUnderCursor(ECC_Visibility, false, hit))
	{
		FVector pos = hit.ImpactPoint;
		pos.Z = GetActorLocation().Z;
		return pos;
	}
	
	// 히트 실패 -> 스크린 -> 월드 역투영
	FVector worldPos, worldDir;
	if (pc->DeprojectMousePositionToWorld(worldPos, worldDir))
	{
		const float charZ = GetActorLocation().Z;
		if (FMath::Abs(worldDir.Z) > KINDA_SMALL_NUMBER)
		{
			const float t = (charZ - worldPos.Z) / worldDir.Z;
			return worldPos + worldDir * t;
		}
	}
	return GetActorLocation() + GetActorForwardVector() * 300.f;
}

FVector AAimi::GetAimDirection() const
{
	FVector dir = GetCursorWorldPosition() - GetActorLocation();
	dir.Z = 0.f;
	return dir.GetSafeNormal();
}

// ════════════════════════════════════════════════════════════
//  DrawAimIndicator — 플래그 기반 분기
// ════════════════════════════════════════════════════════════
void AAimi::DrawAimIndicator()
{
#if ENABLE_DRAW_DEBUG
	LOG_GT(TEXT("DrawAimIndicator — P:%d S:%d Sp:%d"), bAimingPrimary, bAimingSecondary, bAimingSpecial);
#else
	LOG_GT(TEXT("ENABLE_DRAW_DEBUG is OFF!"));
#endif
	
#if ENABLE_ANIM_DEBUG
	// 오브 비행 중 -> 플래그 무관, 항상 폭발 범위 표시
	if (ActiveOrb && !ActiveOrb->HasDetonated())
	{
		DrawGlitchPopAim();
		return;
	}
	
	// Ready() 에서 켠 플래그에 따라 해당 스킬만
	if (bAimingPrimary) DrawGlitchPopAim();
	if (bAimingSecondary) DrawCyberSwipeAim();
	if (bAimingSpecial) DrawSentryAim();
#endif
}

// ════════════════════════════════════════════════════════════
//  DrawGlitchPopAim — 글리치.팝 궤적 + 폭발 범위
// ════════════════════════════════════════════════════════════
void AAimi::DrawGlitchPopAim()
{
#if ENABLE_ANIM_DEBUG
	const FVector charPos = GetActorLocation();
	
	// 오브 활성 -> 오브 주변 폭발 범위
	if (ActiveOrb && !ActiveOrb->HasDetonated())
	{
		const FVector orbPos = ActiveOrb->GetActorLocation();
		const float orbR = ActiveOrb->GetCurrentRadius();
		const float explosionR = orbR * ActiveOrb->ExplosionRadiusMultiplier;
		
		DrawGroundCircle(orbPos, orbR, FColor::White, 24, 2.f);
		DrawGroundCircle(orbPos, explosionR, FColor::Cyan, 24, 1.5f);
		DrawDebugLine(GetWorld(), charPos, orbPos, FColor(0, 200, 255, 100), false, 0.f, 0, 1.f);
		
		// 범위 안 코어에 Push 방향 표시
		TArray<FOverlapResult> overlaps;
		FCollisionShape sphere = FCollisionShape::MakeSphere(explosionR);
		FCollisionQueryParams params;
		params.AddIgnoredActor(this);
		params.AddIgnoredActor(ActiveOrb);
		GetWorld()->OverlapMultiByChannel(overlaps, orbPos, FQuat::Identity, ECC_Pawn, sphere, params);
		
		for (const FOverlapResult& overlap : overlaps)
		{
			AActor* target = overlap.GetActor();
			if (!target || !!target->ActorHasTag(TEXT("Core"))) continue;
			FVector pushDire = (target->GetActorLocation() - orbPos);
			pushDire.Z = 0.f;
			pushDire.Normalize();
			DrawDebugDirectionalArrow(GetWorld(), target->GetActorLocation(), target->GetActorLocation() + pushDire * 150.f, 15.f, FColor::Yellow, false, 0.f, 0, 3.f);
		}
		return;
	}
	// 오브 없음 -> 발사 방향선 (원작처럼 솔리드 라인)
	const FVector aimDir = GetAimDirection();
	if (aimDir.IsNearlyZero()) return;
		
	const float maxDist = 1200.f;
	DrawDebugLine(GetWorld(), charPos, charPos + aimDir * maxDist, FColor(0, 230, 255, 140), false, 0.f, 0, 4.f);
		
	// 최대 거리 폭발 범위
	const FVector endPoint = charPos + aimDir * maxDist;
	DrawGroundCircle(endPoint, 100.f * 2.5f, FColor(0, 200, 255, 100), 32, 1.5f);
	
	// 커서 십자선
	const FVector cursorPos = GetCursorWorldPosition();
	const float cs = 15.f;
	DrawDebugLine(GetWorld(), cursorPos + FVector(-cs, 0, 0), cursorPos + FVector(cs, 0, 0), FColor::White, false, 0.f, 0, 1.5f);
	DrawDebugLine(GetWorld(), cursorPos + FVector(0, -cs, 0), cursorPos + FVector(0, cs, 0), FColor::White, false, 0.f, 0, 1.5f);
#endif
	
}

// ════════════════════════════════════════════════════════════
//  DrawCyberSwipeAim — 점멸 도착지 + 부채꼴
// ════════════════════════════════════════════════════════════
void AAimi::DrawCyberSwipeAim()
{
#if ENABLE_ANIM_DEBUG
	const FVector charPos = GetActorLocation();
	const FVector aimDir = GetAimDirection();
	const FVector blinkEnd = charPos + aimDir * BlinkDistance;
	
	// 점멸 궤적 (보라색 솔리드)
	DrawDebugLine(GetWorld(), charPos, blinkEnd, FColor(200, 100, 255, 160), false, 0.f, 0, 3.f);
	
	// 도착지 원
	DrawGroundCircle(blinkEnd, 25.f, FColor(200, 100, 255, 180), 16, 2.f);
	
	// 도착지 부채꼴 범위
	DrawArcOnGround(blinkEnd, aimDir, SwipeRange, SwipeHalfAngle, FColor(200, 100, 255, 180), 20, 2.f);
#endif
	
}

// ════════════════════════════════════════════════════════════
//  DrawSentryAim — 설치 위치 + 발사 방향
// ════════════════════════════════════════════════════════════
void AAimi::DrawSentryAim()
{
#if ENABLE_ANIM_DEBUG
	const FVector aimDir = GetAimDirection();
	const FVector placePos = GetActorLocation() + aimDir * 80.f;
	
	DrawGroundCircle(placePos, 30.f, FColor::Magenta, 16, 2.f);
	DrawDebugDirectionalArrow(GetWorld(), placePos, placePos + aimDir * 200.f, 15.f, FColor::Magenta, false, 0.f, 0, 2.f);
#endif
	
}

// ════════════════════════════════════════════════════════════
//  DrawGroundCircle / DrawArcOnGround — 유틸
// ════════════════════════════════════════════════════════════
void AAimi::DrawGroundCircle(const FVector& Center, float Radius, const FColor& Color, int32 Segments,
	float Thickness) const
{
#if ENABLE_ANIM_DEBUG
	if (!GetWorld()) return;
	const float step = 2.f * PI / Segments;
	FVector prev = Center + FVector(Radius, 0.f, 0.f);
	for (int32 i = 1; i <= Segments; i++)
	{
		const float a = step * i;
		const FVector next = Center + FVector(FMath::Cos(a) * Radius, FMath::Sin(a) * Radius, 0.f);
		DrawDebugLine(GetWorld(), prev, next, Color, false, 0.f, 0, Thickness);
		prev = next;
	}
#endif
}

void AAimi::DrawArcOnGround(const FVector& Center, const FVector& Forward, float Radius, float HalfAngleDeg,
	const FColor& Color, int32 Segments, float Thickness) const
{
#if ENABLE_ANIM_DEBUG
	if (!GetWorld()) return;
	const float halfRad = FMath::DegreesToRadians(HalfAngleDeg);
	const float step = (2.f * halfRad) / Segments;
	const float startAngle = FMath::Atan2(Forward.Y, Forward.X) - halfRad;
	
	FVector arcStart = Center + FVector(FMath::Cos(startAngle) * Radius, FMath::Sin(startAngle) * Radius, 0.f);
	DrawDebugLine(GetWorld(), Center, arcStart, Color, false, 0.f, 0, Thickness);
	
	FVector prev = arcStart;
	for (int32 i = 1; i <= Segments; i++)
	{
		const float a = startAngle + step * i;
		const FVector next = Center + FVector(FMath::Cos(a) * Radius, FMath::Sin(a) * Radius, 0.f);
		DrawDebugLine(GetWorld(), prev, next, Color, false, 0.f, 0, Thickness);
		prev = next;
	}
	DrawDebugLine(GetWorld(), prev, Center, Color, false, 0.f, 0, Thickness);
#endif
	
}
