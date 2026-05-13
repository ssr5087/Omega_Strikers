// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/CoreBall.h"

#include "PlayerBase.h"
#include "Components/SphereComponent.h"
#include "Core/GoalZone.h"
#include "Net/UnrealNetwork.h"
#include "Omega_Strikers/Omega_Strikers.h"


class AGoalZone;

ACoreBall::ACoreBall()
{
	PrimaryActorTick.bCanEverTick = true;
	
	// ═══════════════════════════════════════════
	// 네트워크 기본 설정
	// ═══════════════════════════════════════════
	bReplicates = true;
	bAlwaysRelevant = true; // Core는 모든 클라이언트에 항상 보여야 함
	SetReplicatingMovement(false); // 커스텀 리플리케이션 사용
 
	// ═══════════════════════════════════════════
	// 콜리전 구체
	// ═══════════════════════════════════════════
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->InitSphereRadius(40.f);
	SphereComp->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	SphereComp->SetGenerateOverlapEvents(true);
	RootComponent = SphereComp;
 
	// ═══════════════════════════════════════════
	// 비주얼 메시
	// ═══════════════════════════════════════════
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(SphereComp);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->SetWorldLocation(FVector(0.0f, 0.0f, -SphereComp->GetScaledSphereRadius()));
	
	ConstructorHelpers::FObjectFinder<USkeletalMesh> tempCoreSKM(TEXT("/Script/Engine.SkeletalMesh'/Game/GT/Environments/Rock/SK_Rock_Default/SkeletalMeshes/SK_Rock_Default.SK_Rock_Default'"));
	if (tempCoreSKM.Succeeded())
	{
		MeshAsset = tempCoreSKM.Object;
		MeshComp->SetSkeletalMesh(MeshAsset);
	}
}

// ═══════════════════════════════════════════════════════
// Replicated 변수 등록
// ═══════════════════════════════════════════════════════
void ACoreBall::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	// 위치/속도: 빠르게 갱신, 조건부로 소유자 제외 없이 전체 전송
	DOREPLIFETIME_CONDITION(ACoreBall, Rep_Location, COND_None);
	DOREPLIFETIME_CONDITION(ACoreBall, Rep_Velocity, COND_None);
	
	// 상태 정보
	DOREPLIFETIME(ACoreBall, Rep_CoreState);
	DOREPLIFETIME(ACoreBall, Rep_HitEvent);
	DOREPLIFETIME(ACoreBall, Rep_GoalEvent);
	DOREPLIFETIME(ACoreBall, Rep_HitCount);
	DOREPLIFETIME(ACoreBall, Rep_SpeedRatio);
}

void ACoreBall::BeginPlay()
{
	Super::BeginPlay();
	
	// 골대 오버랩 바인딩
	SphereComp->OnComponentBeginOverlap.AddDynamic(this, &ACoreBall::OnSphereOverlap);
	
	// 초기 위치 저장
	Rep_Location = GetActorLocation();
	HomeLocation = GetActorLocation();
	ClientTargetLocation = GetActorLocation();
}

// ═══════════════════════════════════════════════════════
// Tick — 서버/클라이언트 분기
// ═══════════════════════════════════════════════════════
void ACoreBall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (HasAuthority())
	{
		// ** 서버: 실제 물리 계산
		ServerPhysicsTick(DeltaTime);
	}
	else
	{
		// ** 클라: 보간만 수행
		ClientInterpolationTick(DeltaTime);
	}
}

// ═══════════════════════════════════════════════════════
// OnRep 콜백 — 클라이언트에서 호출
// ═══════════════════════════════════════════════════════
void ACoreBall::OnRep_ReplicatedLocation()
{
	// 서버에서 새 위치가 도착하면 보간 타겟 갱신
	ClientTargetLocation = Rep_Location;
	ClientVelocity = Rep_Velocity;
	bHasReceivedFirstUpdate = true;
}

void ACoreBall::OnRep_CoreState()
{
	// 클라이언트에서 상태 변경에 따른 비주얼 업데이트
	switch (Rep_CoreState)
	{
	case ECoreState::Idle:
		SetActorHiddenInGame(false);
		SetActorEnableCollision(true);
		// 대기 상태 비주얼 (글로우 끄기 등)
		break;
 
	case ECoreState::InPlay:
		SetActorHiddenInGame(false);
		SetActorEnableCollision(true);
		// 이동 중 비주얼 (트레일 활성화)
		break;
 
	case ECoreState::Stunned:
		SetActorHiddenInGame(false);
		SetActorEnableCollision(true);
		// 스턴 VFX 재생
		break;
 
	case ECoreState::Scored:
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);
		// 골 연출 VFX
		break;
	}
}

void ACoreBall::OnRep_HitEvent()
{
	// 히트 이펙트 재생 (클라이언트)
	// Multicast와 별개로, 놓친 클라이언트가 나중에 받을 수 있도록 보험용
	OnCoreHit.Broadcast(Rep_HitEvent.HitDirection, Rep_HitEvent.Power);
}

void ACoreBall::OnRep_GoalEvent()
{
	if (Rep_GoalEvent.ScoringTeam >= 0)
	{
		OnGoalScored.Broadcast(Rep_GoalEvent.ScoringTeam);
	}
}

// ═══════════════════════════════════════════════════════
// Server RPC — 히트 요청 (클라이언트 → 서버)
// ═══════════════════════════════════════════════════════
void ACoreBall::Server_HitCore_Implementation(FVector HitOrigin, FVector HitDirection, float Power)
{
	// ─── 서버 측 추가 검증 ───
	LOG_GT(TEXT("[CoreBall] Server_HitCore — Power:%.0f, Dir:%s"), Power, *HitDirection.ToString());
 
	// 1. 거리 체크: 히트 원점이 Core로부터 너무 멀면 무시
	float DistToCore = FVector::Dist2D(HitOrigin, GetActorLocation());
	if (DistToCore > MaxHitDistance)
	{
		UE_LOG(LogTemp, Warning, TEXT("CoreBall: Hit rejected - too far (%.1f > %.1f)"),
			DistToCore, MaxHitDistance);
		return;
	}
 
	// 2. 쿨다운 체크
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastHitTime < HitCooldown)
	{
		UE_LOG(LogTemp, Warning, TEXT("CoreBall: Hit rejected - cooldown"));
		return;
	}
	LastHitTime = CurrentTime;
 
	// 3. 상태 체크: Scored 상태면 히트 불가
	if (Rep_CoreState == ECoreState::Scored)
	{
		return;
	}
 
	// ─── 검증 통과 → 힘 적용 ───
	FVector NormalizedDir = HitDirection.GetSafeNormal2D();
	ApplyHitForce(NormalizedDir, Power);
 
	// ─── Multicast VFX ───
	Multicast_PlayHitFX(GetActorLocation(), NormalizedDir, Power);
}

bool ACoreBall::Server_HitCore_Validate(FVector HitOrigin, FVector HitDirection, float Power)
{
	// ─── 기본 검증 ───
 
	// 파워 범위 체크 (파워 상한선)
	if (Power <= 0.f || Power > 50000.f)
	{
		return false;
	}
 
	// 방향 벡터 유효성
	if (HitDirection.IsNearlyZero())
	{
		return false;
	}
 
	return true;
}

// ═══════════════════════════════════════════════════════
// Multicast RPC — VFX 재생
// ═══════════════════════════════════════════════════════
void ACoreBall::Multicast_PlayHitFX_Implementation(FVector Location, FVector Direction, float Power)
{
	// TODO: Niagara VFX 재생
	// 1차에서 만든 Trail, Glow, Impact 파티클을 여기서 트리거
	// UNiagaraFunctionLibrary::SpawnSystemAtLocation(...)
}

void ACoreBall::Multicast_PlayGoalFX_Implementation(int32 ScoringTeam)
{
	// TODO: 골 연출 VFX 재생
	// 팀 색상에 따라 파티클 색 변경
}

// ═══════════════════════════════════════════════════════
// 서버 전용 — 힘 적용
// ═══════════════════════════════════════════════════════
void ACoreBall::ApplyHitForce(FVector Direction, float Power)
{
	LOG_GT(TEXT("Dir: %s, Power: %.1f, Velocity BEFORE: %s"), *Direction.ToString(), Power, *ServerVelocity.ToString());
	
	if (!HasAuthority()) return;
 
	// ─── 연속 타격 보너스 ───
	Rep_HitCount++;
	HitCountResetTimer = 2.0f; // 2초 내 추가 타격 없으면 리셋
 
	// 연속 타격 배수: 1.0 → 1.2 → 1.4 → 1.6 (최대 1.6배)
	float HitMultiplier = FMath::Min(1.0f + (Rep_HitCount - 1) * 0.2f, 1.6f);
 
	// ─── 속도 적용 ───
	FVector Force = Direction.GetSafeNormal2D() * Power * HitMultiplier;
	ServerVelocity += Force;
	ServerVelocity.Z = 0.f;
 
	ClampVelocity();
 
	// ─── 상태 전이 ───
	if (Rep_CoreState == ECoreState::Idle || Rep_CoreState == ECoreState::Stunned)
	{
		Rep_CoreState = ECoreState::InPlay;
	}
 
	// ─── 히트 이벤트 갱신 (OnRep 트리거) ───
	Rep_HitEvent.HitLocation = GetActorLocation();
	Rep_HitEvent.HitDirection = Direction;
	Rep_HitEvent.Power = Power;
	Rep_HitEvent.HitCounter++; // 값 변경 → OnRep 호출
 
	// 서버에서도 델리게이트 호출
	OnCoreHit.Broadcast(Direction, Power);
}

// ═══════════════════════════════════════════════════════
// 리셋 (서버 전용)
// ═══════════════════════════════════════════════════════
void ACoreBall::ResetToCenter()
{
	if (!HasAuthority()) return;
 
	ServerVelocity = FVector::ZeroVector;
	Rep_HitCount = 0;
	Rep_SpeedRatio = 0.f;
	Rep_CoreState = ECoreState::Idle;
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);

	SetActorLocation(HomeLocation);
	Rep_Location = HomeLocation;
	Rep_Velocity = FVector::ZeroVector;
	ClientTargetLocation = HomeLocation;
}

void ACoreBall::SetHomeLocation(const FVector& NewHomeLocation)
{
	if (!HasAuthority()) return;

	HomeLocation = NewHomeLocation;
}

// ═══════════════════════════════════════════════════════
// 서버 물리 Tick
// ═══════════════════════════════════════════════════════
void ACoreBall::ServerPhysicsTick(float DeltaTime)
{
	if (Rep_CoreState == ECoreState::Scored || Rep_CoreState == ECoreState::Stunned) return; // 골 연출 중이거나 스턴 상태면 물리 정지
	
	// 마찰 적용
	ApplyFriction(DeltaTime);
	
	// 정지 처리
	if (ServerVelocity.SizeSquared() < StopThreshold * StopThreshold)
	{
		ServerVelocity = FVector::ZeroVector;
		if (Rep_CoreState == ECoreState::InPlay)
		{
			Rep_CoreState = ECoreState::Idle;
		}
	}
	
	// sweep 이동 (벽 충돌 감지)
	if (!ServerVelocity.IsNearlyZero())
	{
		FVector delta = ServerVelocity * DeltaTime;
		FHitResult hit;
		
		// 2D 물리: Z축 고정
		delta.Z = 0.f;
		
		bool bHit = GetWorld()->SweepSingleByChannel(
			hit,
			GetActorLocation(),
			GetActorLocation() + delta,
			FQuat::Identity,
			ECC_WorldStatic,
			FCollisionShape::MakeSphere(SphereComp->GetScaledSphereRadius()));
		
		// 플레이어랑 겹쳐있는 경우는 무시
		auto* player = Cast<APlayerBase>(hit.GetActor());
		if (player) bHit = false;
		
		if (bHit && hit.bBlockingHit)
		{
			// 충돌 지점까지 이동
			SetActorLocation(hit.Location);
			HandleWallBounce(hit);
		}
		else
		{
			SetActorLocation(GetActorLocation() + delta);
		}
		
		// Z축 고정
		FVector Loc = GetActorLocation();
		Loc.Z = Rep_Location.Z; // 초기 높이 유지
		SetActorLocation(Loc);
	}
	
	// 속도 제한
	ClampVelocity();
	
	// 연속 타격 리셋 타이머
	if (Rep_HitCount > 0)
	{
		HitCountResetTimer -= DeltaTime;
		if (HitCountResetTimer <= 0.f)
		{
			Rep_HitCount = 0;
		}
	}
	
	// 속도 비율 업데이트 (VFX용)
	UpdateSpeedRatio();
	
	// Replicated 변수 갱신
	Rep_Location = GetActorLocation();
	Rep_Velocity = ServerVelocity;
}

void ACoreBall::ApplyFriction(float DeltaTime)
{
	// 프레임 독립적 마찰 : Friction ^ (dt * 60)
	float frameFriction = FMath::Pow(Friction, DeltaTime * 60.f);
	ServerVelocity *= frameFriction;
}

void ACoreBall::HandleWallBounce(const FHitResult& Hit)
{
	// 반사 벡터 계산
	FVector reflected = FMath::GetReflectionVector(ServerVelocity.GetSafeNormal(), Hit.ImpactNormal);
	
	float speed = ServerVelocity.Size() * BounceCoefficient;
	ServerVelocity = reflected * speed;
	ServerVelocity.Z = 0.f; // 2D 고정
}

void ACoreBall::ClampVelocity()
{
	ServerVelocity.Z = 0.f;
	if (ServerVelocity.SizeSquared() > MaxSpeed * MaxSpeed)
	{
		ServerVelocity = ServerVelocity.GetSafeNormal() * MaxSpeed;
	}
}

void ACoreBall::UpdateSpeedRatio()
{
	float currentSpeed = ServerVelocity.Size();
	Rep_SpeedRatio = FMath::Clamp(currentSpeed, 0.f, 1.f);
}

// ═══════════════════════════════════════════════════════
// 클라이언트 보간 Tick
// ═══════════════════════════════════════════════════════
void ACoreBall::ClientInterpolationTick(float DeltaTime)
{
	if (!bHasReceivedFirstUpdate)
	{
		return;
	}
 
	// ─── 예측: 서버 속도 기반으로 타겟 위치 추정 ───
	FVector PredictedTarget = ClientTargetLocation +
		ClientVelocity * DeltaTime;
 
	// ─── 현재 위치 → 예측 타겟으로 보간 ───
	FVector CurrentLoc = GetActorLocation();
	float Distance = FVector::Dist(CurrentLoc, PredictedTarget);
 
	if (Distance > SnapThreshold)
	{
		// 너무 멀면 즉시 텔레포트
		SetActorLocation(PredictedTarget);
	}
	else
	{
		// 부드러운 보간
		FVector NewLoc = FMath::VInterpTo(
			CurrentLoc, PredictedTarget, DeltaTime, InterpSpeed);
		SetActorLocation(NewLoc);
	}
 
	// 타겟도 속도에 따라 전진
	ClientTargetLocation = PredictedTarget;
}

// ═══════════════════════════════════════════════════════
// 골대 오버랩 (서버 전용 처리)
// ═══════════════════════════════════════════════════════
void ACoreBall::OnSphereOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;
 
	// GoalZone과 오버랩 확인
	AGoalZone* Goal = Cast<AGoalZone>(OtherActor);
	if (!Goal) return;
 
	// 이미 득점 상태면 무시
	if (Rep_CoreState == ECoreState::Scored) return;
 
	// 팀 스코어 
	int32 Team = Goal->GetScoringTeam();
 
	// ─── 골 처리 ───
	Rep_CoreState = ECoreState::Scored;
	ServerVelocity = FVector::ZeroVector;
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
 
	// 골 이벤트 갱신 (OnRep 트리거)
	Rep_GoalEvent.ScoringTeam = Team;
	Rep_GoalEvent.GoalCounter++;
 
	// Multicast VFX
	Multicast_PlayGoalFX(Team);
 
	// 델리게이트 (서버)
	OnGoalScored.Broadcast(Team);
 
	UE_LOG(LogTemp, Log, TEXT("CoreBall: GOAL! Team %d scored"), Team);
}

bool ACoreBall::ReceiveImpact_Implementation(const FOSImpactData& ImpactData, AActor* InstigatorActor)
{
	LOG_GT(TEXT("ReceiveImpact - TeamSide: %s, Direction: (%.0f, %.0f), CoreKB: %.0f, PlayerKB: %.0f, PlayerDamage: %.0f"), *StaticEnum<EOSTeam>()->GetValueAsString(ImpactData.TeamSide), ImpactData.Direction.X, ImpactData.Direction.Y, ImpactData.CoreKnockbackPower, ImpactData.PlayerKnockbackPower, ImpactData.PlayerDamage);
	LOG_GT(TEXT("ReceiveImpact - HasAuth: %d, Power: %.0f, State: %s"), HasAuthority(), ImpactData.CoreKnockbackPower, *UEnum::GetValueAsString(Rep_CoreState));
	// ImpactData의 넉백 방향, 충격량 사용
	// 서버에서만 물리 적용
	if (!HasAuthority()) return false;
	
	// Scored 상태면 무시
	if (Rep_CoreState == ECoreState::Scored) return false;
	
	// FVector2D -> FVector 변환 (2D이므로 Z=0)
	FVector knockbackDir = FVector(ImpactData.Direction.X, ImpactData.Direction.Y, 0.f).GetSafeNormal();
	
	// 유효성 체크
	if (knockbackDir.IsNearlyZero() || ImpactData.CoreKnockbackPower <= 0.f) return false;
	
	// 기존 ApplyHitForce 재사용 - 연속 타격 보너스, 상태 전이, 히트 이벤트 리플리케이션 모두 포함
	ApplyHitForce(knockbackDir, ImpactData.CoreKnockbackPower);
	
	// Multicast VFX
	Multicast_PlayHitFX(GetActorLocation(), knockbackDir, ImpactData.CoreKnockbackPower);
	
	return true;
}
