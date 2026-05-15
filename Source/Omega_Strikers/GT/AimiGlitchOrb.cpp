// Fill out your copyright notice in the Description page of Project Settings.


#include "AimiGlitchOrb.h"

#include "NiagaraComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Omega_Strikers/Omega_Strikers.h"
#include "NiagaraFunctionLibrary.h"
#include "PlayerBase.h"
#include "Core/CoreBall.h"
#include "Engine/OverlapResult.h"
#include "Omega_Strikers/SM/OSImpactReceiver.h"

AAimiGlitchOrb::AAimiGlitchOrb()
{
	PrimaryActorTick.bCanEverTick = true;

	// Collision
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->SetSphereRadius(InitRadius);
	CollisionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	CollisionSphere->SetGenerateOverlapEvents(false); // 폭발 시에만 수동 체크
	RootComponent = CollisionSphere;
	
	// Visual (BP에서 Niagara/Mesh 교체 가능)
	OrbMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OrbMesh"));
	OrbMesh->SetupAttachment(RootComponent);
	OrbMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// ★ VFX 컴포넌트 — 오브에 직접 부착
	TrailFXComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TrailFX"));
	TrailFXComp->SetupAttachment(RootComponent);
	TrailFXComp->SetAutoActivate(false);
	
	// Projectile Movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 0.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = false;
	ProjectileMovement->ProjectileGravityScale = 0.f; // 탑뷰 -> 중력 없음
	ProjectileMovement->bShouldBounce = false;

	// 기본값
	CurrentRadius = InitRadius;
}

void AAimiGlitchOrb::BeginPlay()
{
	Super::BeginPlay();
	CurrentRadius = InitRadius;
	UpdateRadius(CurrentRadius);
	
	// ★ 다이나믹 머티리얼 생성
	if (GlitchMaterial && OrbMesh)
	{
		OrbMID = UMaterialInstanceDynamic::Create(GlitchMaterial, this);
		OrbMesh->SetMaterial(0, OrbMID);
	}

	// ★ 나이아가라 에셋 할당 + 활성화
	if (TrailVFX && TrailFXComp)
	{
		TrailFXComp->SetAsset(TrailVFX);
		// Launch() 때 켜기
	}

	if (AuraVFX && AuraFXComp)
	{
		AuraFXComp->SetAsset(AuraVFX);
		// Launch() 때 켜기
	}
}

// ════════════════════════════════════════════════════════════
//  Launch — 캐릭터에서 호출
// ════════════════════════════════════════════════════════════

void AAimiGlitchOrb::Launch(AActor* InOwner, const FVector& Direction)
{
	OwnerCharacter = InOwner;
	MoveDirection = Direction.GetSafeNormal();
	LaunchOrigin = GetActorLocation();
	TraveledDistance = 0.f;

	// ProjectileMovement에 속도 설정
	ProjectileMovement->Velocity = MoveDirection * OrbSpeed;

	// ★ VFX 활성화
	if (TrailFXComp && TrailFXComp->GetAsset())
	{
		TrailFXComp->Activate();
	}
	if (AuraFXComp && AuraFXComp->GetAsset())
	{
		AuraFXComp->Activate();
		AuraFXComp->SetNiagaraVariableFloat(FString("User.OrbRadius"), CurrentRadius);
	}
	
	LOG_GT(TEXT("Launched - Dir: %s, Speed: %.0f"), *MoveDirection.ToString(), OrbSpeed);
}

// ════════════════════════════════════════════════════════════
//  Tick — 성장 + 최대 거리 체크
//
//  ★ 반지름 성장 공식: r(t) = min(MaxRadius, InitialRadius + t × GrowthRate)
// ════════════════════════════════════════════════════════════

void AAimiGlitchOrb::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bDetonated) return;

	// 1) 반지름 성장
	CurrentRadius = FMath::Min(MaxRadius, CurrentRadius + GrowthRate * DeltaTime);
	UpdateRadius(CurrentRadius);

	// 2) 이동 거리 누적
	TraveledDistance = FVector::Dist(GetActorLocation(), LaunchOrigin);

	// 3) 최대 거리 도달 -> 자동 폭발
	if (TraveledDistance >= MaxTravelDistance)
	{
		LOG_GT(TEXT("Max distance reached (%.0f) - auto detonating"), TraveledDistance);
		Detonate();
	}
}

// ════════════════════════════════════════════════════════════
//  Detonate — 재시전 또는 자동 폭발
// ════════════════════════════════════════════════════════════
void AAimiGlitchOrb::Detonate()
{
	if (bDetonated) return;
	bDetonated = true;

	// 이동 정지
	ProjectileMovement->StopMovementImmediately();

	// ★ VFX 정리
	if (TrailFXComp && TrailFXComp->IsActive())
	{
		TrailFXComp->Deactivate();
	}
	if (AuraFXComp && AuraFXComp->IsActive())
	{
		AuraFXComp->Deactivate();
	}

	// 메시 숨기기 (폭발 VFX가 대체)
	if (OrbMesh)
	{
		OrbMesh->SetVisibility(false);
	}
	
	// VFX
	if (DetonateVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), DetonateVFX, GetActorLocation(), FRotator::ZeroRotator, FVector(CurrentRadius / InitRadius));
	}

	// 폭발 판정
	ExecuteExplosion();

	LOG_GT(TEXT("Detonated at %s - Radius: %.0f, ExplosionR: %.0f"), *GetActorLocation().ToString(), CurrentRadius, CurrentRadius * ExplosionRadiusMultiplier);

	// 소멸 (약간의 딜레이로 VFX 재생 보장)
	SetLifeSpan(0.3f);
}

// ════════════════════════════════════════════════════════════
//  ExecuteExplosion
//
//  ★ 핵심 수학 — Explosion Push Vector
//
//  1. 오브 중심 기준 구형 오버랩으로 범위 내 Actor 수집
//  2. 각 대상에 대해:
//       PushDir = normalize(TargetPos - OrbCenter)
//       Force   = BasePower × (1 - dist/explosionRadius × 0.4)
//  3. IOSImpactReceiver를 통해 FOSImpactData 전달
//
//  → 단순 투사체 충돌과 다르게, 폭발 중심에서 방사형으로 밀어냄
//  → 오브 위치를 어디서 터뜨리느냐에 따라 코어/적 방향이 달라짐 (전략적 메카닉)
// ════════════════════════════════════════════════════════════
void AAimiGlitchOrb::ExecuteExplosion()
{
	const FVector orbCenter = GetActorLocation();
	const float explosionRadius = CurrentRadius * ExplosionRadiusMultiplier;

	// 1) 구형(Sphere) 오버랩
	TArray<FOverlapResult> overlaps;
	FCollisionShape sphere = FCollisionShape::MakeSphere(explosionRadius);
	FCollisionQueryParams params;
	params.AddIgnoredActor(this);
	
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
	ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);
	ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	
	if (OwnerCharacter) params.AddIgnoredActor(OwnerCharacter);

	GetWorld()->OverlapMultiByObjectType(overlaps, orbCenter, FQuat::Identity, ObjectParams, sphere, params);

#if WITH_EDITOR
	// 디버그: 폭발 범위 시각화
	//DrawDebugSphere(GetWorld(), orbCenter, explosionRadius, 24, FColor::Magenta, false, 1.5f);
#endif

	for (const FOverlapResult& overlap : overlaps)
	{
		AActor* target = overlap.GetActor();
		LOG_GT(TEXT("Overlap found: %s, Implements IOSImpactReceiver: %d"), 
		*target->GetName(), target->Implements<UOSImpactReceiver>());
		if (!target) continue;
		if (!target->Implements<UOSImpactReceiver>()) continue;

		// ★ 아군 무시
		if (APlayerBase* targetPlayer = Cast<APlayerBase>(target))
		{
			EOSTeam ownerTeam = EOSTeam::None;
			if (APlayerBase* ownerPlayer = Cast<APlayerBase>(OwnerCharacter))
			{
				ownerTeam = ownerPlayer->TeamSide;
			}
			if (targetPlayer->TeamSide == ownerTeam) continue;
		}
		
		// 2) Push Vector 계산
		const FVector targetPos = target->GetActorLocation();
		FVector pushDir = targetPos - orbCenter;
		pushDir.Z = 0.f; // 탑뷰 -> 수평만
		const float distToTarget = pushDir.Size();
		pushDir.Normalize();

		// 거리 감쇠: 가까울수록 강함
		const float attenuation = 1.f - FMath::Clamp(distToTarget / explosionRadius, 0.f, 1.f) * 0.4f;

		// 3) FOSImpactData 구성 (실제 구조체에 맞춤)
		const FVector2D pushDir2D = FVector2D(pushDir.X, pushDir.Y).GetSafeNormal();

		APlayerBase* ownerPlayer = Cast<APlayerBase>(GetOwner());
		if (!ownerPlayer) return;

		FCharacterSkill* skill = ownerPlayer->GetSkillData(TEXT("Aimi_Primary"));
		if (!skill) return;
	
		FOSImpactData data = ownerPlayer->MakeImpactData(*skill);
		
		if (ACoreBall* CoreBall = Cast<ACoreBall>(target))
		{
			FVector KnockDir = FVector(pushDir2D.X, pushDir2D.Y, 0.f).GetSafeNormal();
			CoreBall->Server_HitCore(orbCenter, KnockDir, data.CoreKnockbackPower);
		}
		else
		{
			IOSImpactReceiver::Execute_ReceiveImpact(target, data, OwnerCharacter);
		}
		
		LOG_GT(TEXT("Hit %s - Dir:(%.2f, %.2f) CoreKB:%.0f PlayerKB:%.0f (Attn:%.2f)"), *target->GetName(), pushDir2D.X, pushDir2D.Y, data.CoreKnockbackPower, data.PlayerKnockbackPower, attenuation);

#if WITH_EDITOR
	DrawDebugDirectionalArrow(GetWorld(), orbCenter, orbCenter + pushDir * 200.f, 20.f, FColor::Magenta, false, 1.5f);		
#endif
	}
	
	// 오버랩 자체가 없으면 이것도
	LOG_GT(TEXT("ExecuteExplosion — Overlaps found: %d (ObjectType)"), overlaps.Num());
}

// ════════════════════════════════════════════════════════════
//  UpdateRadius — 콜리전 + 메쉬 스케일 동기 갱신
// ════════════════════════════════════════════════════════════
void AAimiGlitchOrb::UpdateRadius(float NewRadius)
{
	if (CollisionSphere)
	{
		CollisionSphere->SetSphereRadius(NewRadius);
	}

	if (OrbMesh)
	{
		const float scaleFactor = NewRadius / FMath::Max(InitRadius, 1.f);
		OrbMesh->SetWorldScale3D(FVector(scaleFactor));
	}
}

// ════════════════════════════════════════════════════════════
//  ★ UpdateVisualEffects — 매 틱 호출 (신규)
// ════════════════════════════════════════════════════════════
void AAimiGlitchOrb::UpdateVisualEffects()
{
	// 성장 비율 (0~1)
	const float GrowthRatio = FMath::Clamp(
		(CurrentRadius - InitRadius) / FMath::Max(MaxRadius - InitRadius, 1.f),
		0.f, 1.f);

	// ── 다이나믹 머티리얼 ─────────────────────────
	if (OrbMID)
	{
		// 글로우: 커질수록 밝아짐
		OrbMID->SetScalarParameterValue(TEXT("EmissiveIntensity"),
			FMath::Lerp(3.f, 12.f, GrowthRatio));

		// 색상: 시안 → 마젠타 (성장에 따라)
		const FLinearColor OrbColor = FLinearColor::LerpUsingHSV(
			FLinearColor(0.f, 0.9f, 1.f),    // 시안
			FLinearColor(0.8f, 0.2f, 1.f),    // 마젠타
			GrowthRatio);
		OrbMID->SetVectorParameterValue(TEXT("EmissiveColor"), OrbColor);

		// 프레넬: 커질수록 가장자리 글로우 강해짐
		OrbMID->SetScalarParameterValue(TEXT("FresnelPower"),
			FMath::Lerp(3.f, 1.5f, GrowthRatio));

		// 글리치 노이즈 속도: 커질수록 빨라짐
		OrbMID->SetScalarParameterValue(TEXT("NoiseSpeed"),
			FMath::Lerp(1.f, 4.f, GrowthRatio));
	}

	// ── 나이아가라 오라 반지름 ────────────────────
	if (AuraFXComp && AuraFXComp->IsActive())
	{
		AuraFXComp->SetNiagaraVariableFloat(
			FString("User.OrbRadius"), CurrentRadius);

		// 오라 색상도 성장에 따라
		const FLinearColor AuraColor = FLinearColor::LerpUsingHSV(
			FLinearColor(0.f, 0.6f, 1.f, 0.7f),
			FLinearColor(0.6f, 0.1f, 1.f, 0.9f),
			GrowthRatio);
		AuraFXComp->SetNiagaraVariableLinearColor(
			FString("User.AuraColor"), AuraColor);
	}
}

