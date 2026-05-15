// Fill out your copyright notice in the Description page of Project Settings.


#include "Aimi.h"

#include "AimiFirewallSentry.h"
#include "AimiGlitchOrb.h"
#include "DrawDebugHelpers.h"
#include "EngineUtils.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "TimerManager.h"
#include "Core/CoreBall.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Omega_Strikers/Omega_Strikers.h"
#include "Omega_Strikers/SM/HPComponent.h"
#include "Omega_Strikers/SSR/CharacterSkill.h"
#include "AimiAnimInstance.h"
#include "Blueprint/UserWidget.h"
#include "Omega_Strikers/SM/LunaSkillCool.h"

AAimi::AAimi()
{
	PrimaryActorTick.bCanEverTick = true;

	// Power, Speed 등은 PlayerBase에서 관리
	CharacterName = "Aimi";
}

void AAimi::BeginPlay()
{
	Super::BeginPlay();

	//CoreHitCool = 0.f;
	//PrimarySkillCool = 0.f;
	//SecondaryCool = 0.f;
	//SpecialCool = 0.f;

	Energy = 0.f;
	ActiveOrb = nullptr;
	
	// // 데이터 셋
	// FCharacterStat* Stat = GetStatByLevel(Level);
	//
	// if (Stat)
	// {
	// 	ApplyStat(*Stat);
	// }
	// else
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("Stat Load Failed"));
	// }

	LOG_GT(TEXT("[Aimi] NS_AimIndicator valid: %s"), NS_AimIndicator? TEXT("True") : TEXT("False"));

	AimIndicatorComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
		NS_AimIndicator,
		GetRootComponent(),
		NAME_None,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		EAttachLocation::KeepWorldPosition,
		false,
		true
		);

	AimIndicatorComp->SetVisibility(false);
	
	DecalIndicatorComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
		NS_AimIndicator,
		GetRootComponent(),
		NAME_None,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		EAttachLocation::KeepWorldPosition,
		false
		);
	
	// UI 붙이기
	SkillUI = CreateWidget<ULunaSkillCool>(GetWorld(), CoolTimeUI);
	if (SkillUI)
	{
		SkillUI->AddToViewport();
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
	
	DrawAimIndicator();
}

// ════════════════════════════════════════════
//  2. 헬퍼 람다 — 파일 상단 또는 anonymous namespace
//     AnimInstance 캐스팅을 매번 쓰기 번거로우니 인라인 함수로 뺌
// ════════════════════════════════════════════
UAimiAnimInstance* AAimi::GetAimiAnim() const
{
	return Cast<UAimiAnimInstance>(GetMesh()->GetAnimInstance());
}

// ════════════════════════════════════════════════════════════
//  쿨다운
// ════════════════════════════════════════════════════════════
void AAimi::TickCooldowns(float DeltaTime)
{
	//if (CoreHitCool	 > 0.f) CoreHitCool	 = FMath::Max(0.f, CoreHitCool	 - DeltaTime);
	if (PrimarySkillCool   > 0.f) PrimarySkillCool	 = FMath::Max(0.f, PrimarySkillCool	 - DeltaTime);
	if (SecondaryCool > 0.f) SecondaryCool = FMath::Max(0.f, SecondaryCool - DeltaTime);
	if (SpecialCool	 > 0.f) SpecialCool	 = FMath::Max(0.f, SpecialCool   - DeltaTime);
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
	// 좌클릭을 해제하면 PlayerBase에서 알아서 사용
	Super::Ready_CoreHit();
}

void AAimi::Use_CoreHit()
{
	Super::Use_CoreHit();
	
	// 몽타주 재생
	if (UAimiAnimInstance* anim = GetAimiAnim()) anim->PlayStrike();
}

// PlayerBase에서 이미 평타 구현 되어있어서 안쓸 듯...
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
	Super::Ready_PrimarySkill();
	LOG_GT(TEXT("Ready_PrimarySkill called! bAimingPrimary will be set true. PrimarySkillCool: %.1f"), PrimarySkillCool);
	if (PrimarySkillCool > 0.f) return;
	
	if (ActiveOrb && !ActiveOrb->HasDetonated())
	{
		// 재시전 대기 UI
		
	}
}

void AAimi::Use_PrimarySkill()
{
	LOG_GT(TEXT("Use_PrimarySkill called! — bAimingPrimary was %d"), bAimingPrimary);
	Super::Use_PrimarySkill();
	
	if (ActiveOrb && !ActiveOrb->HasDetonated())
	{
		RecastGlitchOrb();
	}
	else
	{
		if (PrimarySkillCool > 0.f) return;
		FireGlitchOrb();
		PrimarySkillCool = GetAdjustedCD(PrimaryCool_Max);
	}
	
}

void AAimi::FireGlitchOrb()
{
	if (!GlitchOrbClass)
	{
		LOG_GT_E(TEXT("GlitchOrbClass not set!"));
		return;
	}

	bIsAimingOrb = true;

	const FVector spawnLoc = GetActorLocation() + CachedAimDirection * 60.f;
	const FVector aimDir = CachedAimDirection;

	FActorSpawnParameters spawnParams;
	spawnParams.Owner = this;
	spawnParams.Instigator = this;
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ActiveOrb = GetWorld()->SpawnActor<AAimiGlitchOrb>(GlitchOrbClass, spawnLoc, aimDir.Rotation(), spawnParams);

	if (ActiveOrb)
	{
		ActiveOrb->Launch(this, aimDir);
		LOG_GT(TEXT("Glitch.Pop - Orb launched"));
		
		// 몽타주 재생
		if (UAimiAnimInstance* anim = GetAimiAnim()) anim->PlayGlitchOrb();
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
	if (SecondaryCool > 0.f) return;
	Super::Ready_SecondarySkill();
}

void AAimi::Use_SecondarySkill()
{
	Super::Use_SecondarySkill();
	if (SecondaryCool > 0.f) return;
	DoCyberSwipe();
	SecondaryCool = GetAdjustedCD(SecondaryCool_Max);
}

void AAimi::DoCyberSwipe()
{
	bIsDashing = true;
	
	FVector blinkDir = CachedAimDirection;
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
	
	// 몽타주 재생
	if (UAimiAnimInstance* anim = GetAimiAnim()) anim->PlayCyberSwipe();
}

void AAimi::OnCyberSwipeArrived()
{
	bIsDashing = false;
	
	const FVector origin = GetActorLocation();
	const FVector forward = GetActorForwardVector();
	const FVector2D dir2D = FVector2D(forward.X, forward.Y).GetSafeNormal();

	FCharacterSkill* Skill = GetSkillData(TEXT("Aimi_Secondary"));
	if (!Skill) return;
		
	FOSImpactData data = MakeImpactData(*Skill);

	PerformSlash(origin, forward, SwipeRange, SwipeHalfAngle, data);
	
	LOG_GT(TEXT("Cyber Swipe - Tail slash!"));
}

// ════════════════════════════════════════════════════════════
//  [Special] 방화벽 파수꾼 — 터렛 설치
// ════════════════════════════════════════════════════════════

void AAimi::Ready_SpecialSkill()
{
	if (SpecialCool > 0.f) return;
	Super::Ready_SpecialSkill();
}

void AAimi::Use_SpecialSkill()
{
	Super::Use_SpecialSkill();
	if (SpecialCool > 0.f) return;
	PlaceSentry();
	SpecialCool = GetAdjustedCD(CD_Special_Max);
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

	const FVector forward = CachedAimDirection;
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
		
		// 몽타주 재생
		if (UAimiAnimInstance* anim = GetAimiAnim()) anim->PlayPlaceSentry();
	}
}

// ════════════════════════════════════════════════════════════
//  [Flip] 에너지 미터
// ════════════════════════════════════════════════════════════
void AAimi::Ready_Flip()
{
	
}

void AAimi::Use_Flip()
{
	Energy >= 100.f ? DoEnergyBurst() : DoDodge();
}

void AAimi::DoDodge()
{
	bIsDashing = true;
	
	FVector dashDir = GetCharacterMovement()->GetLastInputVector();
	if (dashDir.IsNearlyZero()) dashDir = GetActorForwardVector();
	dashDir.Z = 0.f;
	dashDir.Normalize();

	LaunchCharacter(dashDir * DodgeDashForce, true, false);
	LOG_GT(TEXT("Dodge done"));

	// 대시 끝나면 false (0.3f는 대시 지속 시간에 맞게 조절)
	FTimerHandle timer;
	GetWorldTimerManager().SetTimer(timer, [this]()
	{
		bIsDashing = false;
	}, 0.3f, false);
	
	// 몽타주 재생
	if (UAimiAnimInstance* anim = GetAimiAnim()) anim->PlayFlip();
}

void AAimi::DoEnergyBurst()
{
	Energy = 0.f;

	const FVector origin = GetActorLocation();
	const FVector forward = GetActorForwardVector();
	const FVector2D dir2D = FVector2D(forward.X, forward.Y).GetSafeNormal();

	FCharacterSkill* Skill = GetSkillData(TEXT("Aimi_EnergyBurst"));
	if (!Skill) return;
		
	FOSImpactData data = MakeImpactData(*Skill);
	data.Direction = dir2D;
	
	PerformSlash(origin, forward, EnergyBurstRange, 180.f, data);
	LOG_GT(TEXT("ENERGY BURST! 360°"));
	
	// 몽타주 재생
	if (UAimiAnimInstance* anim = GetAimiAnim()) anim->PlayFlip();
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
	// ★ 디버그: CoreBall 직접 찾기
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		if (It->GetClass()->GetName().Contains(TEXT("CoreBall")))
		{
			float Dist = FVector::Dist(GetActorLocation(), It->GetActorLocation());
			UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(It->GetRootComponent());
			LOG_GT(TEXT("[Debug] CoreBall 발견 — 거리:%.0f, ColEnabled:%d, ObjType:%d"),
				Dist,
				Root ? (int32)Root->GetCollisionEnabled() : -1,
				Root ? (int32)Root->GetCollisionObjectType() : -1);
		}
	}
	
	FVector Forward = ForwardDir;
	Forward.Z = 0.f;
	Forward.Normalize();
 
	TArray<FOverlapResult> Overlaps;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(Range);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
	ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);
	ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	
	GetWorld()->OverlapMultiByObjectType(
		Overlaps, Origin, FQuat::Identity,
		ObjectParams, Sphere, Params);
 
#if WITH_EDITOR
	DrawDebugSphere(GetWorld(), Origin, Range, 16, FColor::Orange, false, 0.5f);
#endif
 
	const float CosThreshold = FMath::Cos(FMath::DegreesToRadians(HalfAngleDeg));
	bool bHitAny = false;
 
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Target = Overlap.GetActor();
		LOG_GT(TEXT("Overlap found: %s, Implements IOSImpactReceiver: %d"), 
		*Target->GetName(), Target->Implements<UOSImpactReceiver>());
		if (!Target || !Target->Implements<UOSImpactReceiver>()) continue;
 
		// ★ 아군 무시 — 같은 팀이면 스킵
		if (APlayerBase* TargetPlayer = Cast<APlayerBase>(Target))
		{
			if (TargetPlayer->TeamSide == TeamSide) continue;
		}
		
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
 
		//IOSImpactReceiver::Execute_ReceiveImpact(Target, Data, this);
		// ✅ 수정 — CoreBall이면 Server RPC 사용
		if (ACoreBall* CoreBall = Cast<ACoreBall>(Target))
		{
			FVector KnockDir = FVector(Data.Direction.X, Data.Direction.Y, 0.f).GetSafeNormal();
			CoreBall->Server_HitCore(GetActorLocation(), KnockDir, Data.CoreKnockbackPower);
		}
		else
		{
			IOSImpactReceiver::Execute_ReceiveImpact(Target, Data, this);
		}
		bHitAny = true;
 
		LOG_GT(TEXT("[AiMi] Slash hit %s — Dir:(%.2f,%.2f) CoreKB:%.0f PlayerKB:%.0f Dmg:%.0f"),
			*Target->GetName(), PushDir2D.X, PushDir2D.Y,
			Data.CoreKnockbackPower, Data.PlayerKnockbackPower, Data.PlayerDamage);
	}
 
	// 오버랩 자체가 없으면 이것도
	LOG_GT(TEXT("PerformSlash — Overlaps found: %d (ObjectType, Range: %.0f)"), Overlaps.Num(), Range);
	
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
	if (!IsLocallyControlled()) return;
	
#if ENABLE_ANIM_DEBUG
	// 매 프레인 에이밍 방향 캐싱
	CachedAimDirection = GetAimDirection();
	
	/*// 오브 비행 중 -> 플래그 무관, 항상 폭발 범위 표시
	if (ActiveOrb && !ActiveOrb->HasDetonated())
	{
		DrawGlitchPopAim();
		return;
	}*/
	
	// Ready() 에서 켠 플래그에 따라 해당 스킬만
	//if (bAimingSecondary) DrawCyberSwipeAim();
	//if (bAimingSpecial) DrawSentryAim();
#endif
	if (bAimingPrimary) UpdateAimIndicator();
}

void AAimi::UpdateAimIndicator()
{
	if (!AimIndicatorComp) return;

	// Activate 대신 Visibility로 제어
	AimIndicatorComp->SetVisibility(bAimingPrimary);
	DecalIndicatorComp->SetVisibility(bAimingPrimary);

	if (!bAimingPrimary) return;

	const FVector aimDir3D = FVector(CursorDir.X, CursorDir.Y, 0.f).GetSafeNormal();
	const float dist = 1200.f;

	AimIndicatorComp->SetWorldLocation(GetActorLocation());
	AimIndicatorComp->SetWorldRotation(aimDir3D.Rotation());
	AimIndicatorComp->SetVariableFloat(FName("User.AimDistance"), dist);

	if (DecalIndicatorComp) DecalIndicatorComp->SetWorldLocation(GetActorLocation() + aimDir3D * dist);
}

// ════════════════════════════════════════════════════════════
//  DrawGlitchPopAim — 글리치.팝 궤적 + 폭발 범위
// ════════════════════════════════════════════════════════════
void AAimi::DrawGlitchPopAim()
{
#if ENABLE_ANIM_DEBUG
	FVector charPos = GetActorLocation();
	
	// 오브 활성 -> 오브 주변 폭발 범위
	if (ActiveOrb && !ActiveOrb->HasDetonated())
	{
		FVector orbPos = ActiveOrb->GetActorLocation();
		orbPos.Z = charPos.Z; // ★ 오브 라인도 동일 높이로
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
			if (!target || !target->ActorHasTag(TEXT("Core"))) continue;
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
