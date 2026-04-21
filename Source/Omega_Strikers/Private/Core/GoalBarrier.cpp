// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/GoalBarrier.h"

#include "Core/CoreBall.h"
#include "Net/UnrealNetwork.h"
#include "Omega_Strikers/Omega_Strikers.h"

AGoalBarrier::AGoalBarrier()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	// 메시 콤포넌트
	BarrierMesh = CreateDefaultSubobject<UStaticMeshComponent>("BarrierMesh");
	RootComponent = BarrierMesh;
	
	// 기본 콜리전 세팅
	BarrierMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BarrierMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	BarrierMesh->SetCollisionResponseToAllChannels(ECR_Block);
	BarrierMesh->SetGenerateOverlapEvents(false);
	BarrierMesh->SetNotifyRigidBodyCollision(true); // Hit 이벤트 받기
	BarrierMesh->SetMobility(EComponentMobility::Stationary);
}

void AGoalBarrier::BeginPlay()
{
	Super::BeginPlay();
	
	CurrentHP = MaxHP;
	bIsDestroyed = false;
	SetupCollision();
}

// ═══════════════════════════════════════════════════════
// 네트워크 리플리케이션
// ═══════════════════════════════════════════════════════
void AGoalBarrier::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AGoalBarrier, CurrentHP);
	DOREPLIFETIME(AGoalBarrier, bIsDestroyed);
}

// ═══════════════════════════════════════════════════════
// 콜리전 세팅
// ═══════════════════════════════════════════════════════
void AGoalBarrier::SetupCollision()
{
	// 물리 메테리얼
	if (!BarrierPhysMat)
	{
		BarrierPhysMat = NewObject<UPhysicalMaterial>(this, TEXT("BarrierPhysMat"));
	}
	BarrierPhysMat->Restitution = Restitution;
	BarrierPhysMat->Friction = 0.1f;
	BarrierPhysMat->RestitutionCombineMode = EFrictionCombineMode::Max;
	
	BarrierMesh->SetPhysMaterialOverride(BarrierPhysMat);
	
	if (bUseComplexCollision)
	{
		BarrierMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		// 메시 에디터에서 "Use Complex Collision As Simple" 설정 필요
	}
}

// ═══════════════════════════════════════════════════════
// 코어 히트 처리
// ═══════════════════════════════════════════════════════
void AGoalBarrier::HitByCore()
{
	if (bIsDestroyed) return;
	
	CurrentHP = FMath::Max(0, CurrentHP - 1);
	
	if (CurrentHP <= 0)
	{
		// 파괴
		bIsDestroyed = true;
		
		// 콜리전 끄기
		BarrierMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		BarrierMesh->SetVisibility(false);
		
		// 파괴 FX
		PlayDestroyFX();
		
		// CoreArena 또는 GameMode에 알림
		OnBarrierDestroyed.Broadcast(this);
		
		LOG_GT(TEXT("Team %d : 배리어 파괴됨"), TeamIndex);
	}
	else
	{
		// 히트 FX
		PlayHitFX();
		
		LOG_GT(TEXT("Team %d : 배리어 피격 -> HP=%d/%d"), TeamIndex, CurrentHP, MaxHP);
	}
}

// ═══════════════════════════════════════════════════════
// 리셋 (라운드 시작)
// ═══════════════════════════════════════════════════════
void AGoalBarrier::ResetBarrier()
{
	CurrentHP = MaxHP;
	bIsDestroyed = false;
	
	BarrierMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BarrierMesh->SetVisibility(true);
	
	LOG_GT(TEXT("Team %d: 리셋"), TeamIndex);
}

// ═══════════════════════════════════════════════════════
// 연출 (블루프린트에서 오버라이드 가능)
// ═══════════════════════════════════════════════════════
void AGoalBarrier::PlayDestroyFX_Implementation()
{
	// 기본: 그냥 숨김. BP에서 Niagara 파티클 등 추가 가능.
}

void AGoalBarrier::PlayHitFX_Implementation()
{
	// 기본: 로그만. BP에서 머티리얼 변경, 흔들림 등 추가 가능.
}

// ═══════════════════════════════════════════════════════
// 물리 Hit 감지 — 코어가 부딪혔을 때
// ═══════════════════════════════════════════════════════
void AGoalBarrier::NotifyHit(class UPrimitiveComponent* MyComp, AActor* Other, class UPrimitiveComponent* OtherComp,
	bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);
	
	// 코어인지 체크
	if (!Cast<ACoreBall>(Other)) return;
	
	// 서버에서만 처리
	if (!HasAuthority()) return;
	
	// 이미 파괴된 배리어
	if (bIsDestroyed) return;
	
	HitByCore();
}
