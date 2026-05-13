// Fill out your copyright notice in the Description page of Project Settings.


#include "AimiFirewallSentry.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "PlayerBase.h"
#include "Core/CoreBall.h"
#include "Omega_Strikers/Omega_Strikers.h"
#include "Omega_Strikers/SM/OSImpactReceiver.h"


// Sets default values
AAimiFirewallSentry::AAimiFirewallSentry()
{
	PrimaryActorTick.bCanEverTick = true;

	SentryMesh = CreateDefaultSubobject<UStaticMeshComponent>("SentryMesh");
	RootComponent = SentryMesh;
	SentryMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	
	// ★ 감지 범위 링 나이아가라
	DetectRingComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("DetectRingFX"));
	DetectRingComp->SetupAttachment(RootComponent);
	DetectRingComp->SetAutoActivate(false);
}

void AAimiFirewallSentry::BeginPlay()
{
	Super::BeginPlay();
	CurrentHP = SentryHP;
	
	// ★ 감지 링 에셋 할당
	if (DetectRingVFX && DetectRingComp)
	{
		DetectRingComp->SetAsset(DetectRingVFX);
	}
}

void AAimiFirewallSentry::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bInitialized) return;

	ElapsedTime += DeltaTime;

	// 지속 시간 종료
	if (ElapsedTime >= Duration)
	{
		// ★ 소멸 VFX
		if (DestroyVFX)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(), DestroyVFX, GetActorLocation(),
				GetActorRotation(), FVector(1.f), true, true,
				ENCPoolMethod::AutoRelease);
		}
		
		LOG_GT(TEXT("Duration expired - destroying"));
		Destroy();
		return;
	}

	// 발사 타이머
	FireTimer -= DeltaTime;
	if (FireTimer <= 0.f)
	{
		FireProjectile();
		FireTimer = FireInterval;
	}
}

void AAimiFirewallSentry::Initialize(AActor* IsOwner, const FVector& FireDirection)
{
	OwnerCharacter = IsOwner;
	FireDir = FireDirection.GetSafeNormal();
	FireDir.Z = 0.f;
	FireDir.Normalize();
	bInitialized = true;
	ElapsedTime = 0.f;
	FireTimer = 0.f; // 즉시 첫 발사

	// 터렛이 발사 방향을 바라보도록 설정
	SetActorRotation(FireDir.Rotation());

	// ★ 소환 VFX
	if (SpawnVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), SpawnVFX, GetActorLocation(),
			GetActorRotation(), FVector(1.f), true, true,
			ENCPoolMethod::AutoRelease);
	}

	// ★ 감지 링 활성화
	if (DetectRingComp && DetectRingComp->GetAsset())
	{
		DetectRingComp->Activate();
		DetectRingComp->SetNiagaraVariableFloat(
			FString("User.RingRadius"), ProjectileRange);
	}

	
	LOG_GT(TEXT("Initialized - Dir: %s, Duration: %.1f"), *FireDir.ToString(), Duration);
}

// ════════════════════════════════════════════════════════════
//  FireProjectile — 직선 투사체 or 라인트레이스
// ════════════════════════════════════════════════════════════
void AAimiFirewallSentry::FireProjectile()
{
	const FVector start = GetActorLocation();

	if (ProjectileClass)
	{
		// BP 투사체 스폰 방식
		FActorSpawnParameters spawnParams;
		spawnParams.Owner = this;
		spawnParams.Instigator = Cast<APawn>(OwnerCharacter);
		spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AActor* proj = GetWorld()->SpawnActor<AActor>(ProjectileClass, start, FireDir.Rotation(), spawnParams);

		if (proj)
		{
			// 투사체에 속도를 부여하는 방법은 BP 또는 ProjectileMovement에서 처리
			LOG_GT(TEXT("Fired projectile -> %s"), *proj->GetName());
		}
	}
	else
	{
		// 라인 트레이스 즉발 방식 (ProjectileClass 없을 경우)
		const FVector end = start + FireDir * ProjectileRange;

		FHitResult hit;
		FCollisionQueryParams params;
		params.AddIgnoredActor(this);
		if (OwnerCharacter) params.AddIgnoredActor(OwnerCharacter);

		bool bHit = GetWorld()->LineTraceSingleByChannel(hit, start, end, ECC_Pawn, params);
		
		// ★ VFX 스폰 (디버그 라인 대체)
		SpawnFireVFX(start, end, bHit, bHit ? hit.ImpactPoint : end);

#if WITH_EDITOR
		DrawDebugLine(GetWorld(), start, bHit ? hit.ImpactPoint : end, FColor::Magenta, false, FireInterval * 0.8f, 0, 2.f);
#endif

		if (bHit && hit.GetActor())
		{
			AActor* target = hit.GetActor();
			if (target->Implements<UOSImpactReceiver>())
			{
				const FVector2D dir2D = FVector2D(FireDir.X, FireDir.Y).GetSafeNormal();

				APlayerBase* ownerPlayer = Cast<APlayerBase>(GetOwner());
				if (!ownerPlayer) return;

				FCharacterSkill* skill = ownerPlayer->GetSkillData(TEXT("Aimi_Primary"));
				if (!skill) return;
	
				FOSImpactData data = ownerPlayer->MakeImpactData(*skill);
				data.Direction = dir2D;
				
				// ✅ CoreBall이면 Server RPC 사용
				if (ACoreBall* CoreBall = Cast<ACoreBall>(target))
				{
					FVector KnockDir = FVector(data.Direction.X, data.Direction.Y, 0.f).GetSafeNormal();
					CoreBall->Server_HitCore(start, KnockDir, data.CoreKnockbackPower);
				}
				else
				{
					IOSImpactReceiver::Execute_ReceiveImpact(target, data, this);
				}
				
				LOG_GT(TEXT("Hit %s - CoreKB:%.0f PlayerKB:%0.f"), *target->GetName(), ProjectileCoreKnockback, ProjectilePlayerKnockback);
			}
		}
	}
}

// ════════════════════════════════════════════════════════════
//  ★ SpawnFireVFX — 발사 시 VFX 스폰 (신규)
// ════════════════════════════════════════════════════════════
void AAimiFirewallSentry::SpawnFireVFX(const FVector& Start, const FVector& End, bool bHit, const FVector& HitPoint)
{
	// 머즐플래시
	if (MuzzleFlashVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), MuzzleFlashVFX, Start,
			FireDir.Rotation(), FVector(1.f), true, true,
			ENCPoolMethod::AutoRelease);
	}

	// 발사 궤적 빔 (시작~끝점)
	if (BeamTrailVFX)
	{
		UNiagaraComponent* BeamComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), BeamTrailVFX, Start,
			FireDir.Rotation(), FVector(1.f), true, true,
			ENCPoolMethod::AutoRelease);

		if (BeamComp)
		{
			BeamComp->SetNiagaraVariableVec3(
				FString("User.BeamStart"), Start);
			BeamComp->SetNiagaraVariableVec3(
				FString("User.BeamEnd"), bHit ? HitPoint : End);
			BeamComp->SetNiagaraVariableLinearColor(
				FString("User.BeamColor"),
				FLinearColor(0.2f, 1.f, 0.5f, 1.f)); // 녹색 디지털
		}
	}

	// 히트 임팩트
	if (bHit && HitImpactVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), HitImpactVFX, HitPoint,
			(-FireDir).Rotation(), FVector(1.f), true, true,
			ENCPoolMethod::AutoRelease);
	}
}
