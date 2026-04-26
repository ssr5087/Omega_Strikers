// Fill out your copyright notice in the Description page of Project Settings.


#include "AimiGlitchOrb.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Omega_Strikers/Omega_Strikers.h"
#include "NiagaraFunctionLibrary.h"
#include "PlayerBase.h"
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
	// TODO: 아군 플레이어도 충돌 무시 처리해야함
	params.AddIgnoredActor(this);
	if (OwnerCharacter) params.AddIgnoredActor(OwnerCharacter);

	GetWorld()->OverlapMultiByChannel(overlaps, orbCenter, FQuat::Identity, ECC_Pawn, sphere, params);

#if WITH_EDITOR
	// 디버그: 폭발 범위 시각화
	DrawDebugSphere(GetWorld(), orbCenter, explosionRadius, 24, FColor::Magenta, false, 1.5f);
#endif

	for (const FOverlapResult& overlap : overlaps)
	{
		AActor* target = overlap.GetActor();
		if (!target) continue;
		if (!target->Implements<UOSImpactReceiver>()) continue;

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

		// 오너 캐릭터의 TeamSide 가져오기
		EOSTeam ownerTeam = EOSTeam::None;
		if (APlayerBase* ownerPlayer = Cast<APlayerBase>(OwnerCharacter))
		{
			ownerTeam = ownerPlayer->TeamSide;
		}

		FOSImpactData impactData;
		impactData.TeamSide = ownerTeam;
		impactData.Direction = pushDir2D;
		impactData.CoreKnockbackPower = CoreKnockback * attenuation;
		impactData.PlayerKnockbackPower = PlayerKnockback * attenuation;
		impactData.PlayerDamage = Damage * attenuation;

		IOSImpactReceiver::Execute_ReceiveImpact(target, impactData, OwnerCharacter);

		LOG_GT(TEXT("Hit %s - Dir:(%.2f, %.2f) CoreKB:%.0f PlayerKB:%.0f (Attn:%.2f)"), *target->GetName(), pushDir2D.X, pushDir2D.Y, impactData.CoreKnockbackPower, impactData.PlayerKnockbackPower, attenuation);

#if WITH_EDITOR
	DrawDebugDirectionalArrow(GetWorld(), orbCenter, orbCenter + pushDir * 200.f, 20.f, FColor::Magenta, false, 1.5f);		
#endif
	}
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

