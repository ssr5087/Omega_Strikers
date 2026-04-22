// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/CoreArena.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Engine/StaticMesh.h"
#include "Core/CoreBall.h"
#include "Core/GoalBarrier.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Omega_Strikers/Omega_Strikers.h"

ACoreArena::ACoreArena()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	// 루트 
	ArenaRoot = CreateDefaultSubobject<USceneComponent>(TEXT("ArenaRoot"));
	RootComponent = ArenaRoot;
	
	// ═══════════════════════════════════════════
	// 아레나 메시 컴포넌트 (비주얼 + 콜리전)
	// ═══════════════════════════════════════════
	ArenaMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArenaMesh"));
	ArenaMesh->SetupAttachment(ArenaRoot);
	ArenaMesh->SetMobility(EComponentMobility::Static);
	
	ConstructorHelpers::FObjectFinder<UStaticMesh> tempArenaMesh(TEXT("/Script/Engine.StaticMesh'/Game/GT/Environments/Arena/Arenas/AhtenCity/Meshes/LOD0/FBX/OS_Arena_AhtenCity.OS_Arena_AhtenCity'"));
	
	if (tempArenaMesh.Succeeded())
	{
		ArenaMeshAsset = tempArenaMesh.Object;
	}
	
	// ═══════════════════════════════════════════
	// 다이 벽 메시 컴포넌트 (비주얼 + 콜리전)
	// ═══════════════════════════════════════════
	WallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WallMesh"));
	WallMesh->SetupAttachment(ArenaRoot);
	WallMesh->SetMobility(EComponentMobility::Static);
	
	ConstructorHelpers::FObjectFinder<UStaticMesh> tempWallMesh(TEXT("/Script/Engine.StaticMesh'/Game/GT/Environments/Arena/ArenaMain5/Arena/Meshes/FBX/SM_Arena_5_Measures.SM_Arena_5_Measures'"));
	
	if (tempWallMesh.Succeeded())
	{
		WallMeshAsset = tempWallMesh.Object;
	}
	
	// ═══════════════════════════════════════════
	// 골 트리거 볼륨
	// ═══════════════════════════════════════════
	LeftGoalTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftGoalTrigger"));
	LeftGoalTrigger->SetupAttachment(ArenaRoot);
	LeftGoalTrigger->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	LeftGoalTrigger->SetGenerateOverlapEvents(true);
	LeftGoalTrigger->SetCollisionResponseToAllChannels(ECR_Overlap);
	LeftGoalTrigger->ShapeColor = FColor::Blue;
	
	RightGoalTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("RightGoalTrigger"));
	RightGoalTrigger->SetupAttachment(ArenaRoot);
	RightGoalTrigger->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	RightGoalTrigger->SetGenerateOverlapEvents(true);
	RightGoalTrigger->SetCollisionResponseToAllChannels(ECR_Overlap);
	RightGoalTrigger->ShapeColor = FColor::Red;
	
	// ═══════════════════════════════════════════
	// 골대 비주얼 메시 (트리거에 붙힘)
	// ═══════════════════════════════════════════
	LeftGoalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftGoalMesh"));
	LeftGoalMesh->SetupAttachment(LeftGoalTrigger);
	LeftGoalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	RightGoalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightGoalMesh"));
	RightGoalMesh->SetupAttachment(RightGoalTrigger);
	RightGoalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	ConstructorHelpers::FObjectFinder<UStaticMesh> tempLeftGoalMesh(TEXT("/Script/Engine.StaticMesh'/Game/GT/Environments/Season02/Arena/GoalDesigns/Meshes/FBX/SM_GoalDesign_Overhang_01_Team01.SM_GoalDesign_Overhang_01_Team01'"));
	if (tempLeftGoalMesh.Succeeded())
	{
		LeftGoalMeshAsset = tempLeftGoalMesh.Object;
	}
	
	ConstructorHelpers::FObjectFinder<UStaticMesh> tempRightGoalMesh(TEXT("/Script/Engine.StaticMesh'/Game/GT/Environments/Season02/Arena/GoalDesigns/Meshes/FBX/SM_GoalDesign_Overhang_01_Team02.SM_GoalDesign_Overhang_01_Team02'"));
	if (tempRightGoalMesh.Succeeded())
	{
		RightGoalMeshAsset = tempRightGoalMesh.Object;
	}
	
	// ═══════════════════════════════════════════
	// 넉아웃 경계
	// ═══════════════════════════════════════════
	KnockoutBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("KnockoutBounds"));
	KnockoutBounds->SetupAttachment(ArenaRoot);
	KnockoutBounds->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	KnockoutBounds->ShapeColor = FColor::Yellow;
	
	// 기본값 세팅
	SetupDefaultSpawns();
}

void ACoreArena::BeginPlay()
{
	Super::BeginPlay();
	
	// 골 오버랩 바인딩
	if (LeftGoalTrigger)
	{
		LeftGoalTrigger->OnComponentBeginOverlap.AddDynamic(this, &ACoreArena::OnGoalOverlap);
	}
	
	if (RightGoalTrigger)
	{
		RightGoalTrigger->OnComponentBeginOverlap.AddDynamic(this, &ACoreArena::OnGoalOverlap);
	}
	
	// 레벨에 배치된 GoalBarrier 수집
	CollectBarriers();
	
	// 게이트 닫힌 상태로 시작
	bLeftGateOpen = false;
	bRightGateOpen = false;
}

// ═══════════════════════════════════════════════════════
// 네트워크
// ═══════════════════════════════════════════════════════
void ACoreArena::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ACoreArena, bLeftGateOpen);
	DOREPLIFETIME(ACoreArena, bRightGateOpen);
}

// ═══════════════════════════════════════════════════════
// 기본 스폰 위치 (3v3 기준)
// ═══════════════════════════════════════════════════════
void ACoreArena::SetupDefaultSpawns()
{
	TeamASpawnOffsets.Empty();
	TeamASpawnOffsets.Add(FVector(-900.0f, -2400.0f, 50.0f)); // Forward 상단
	TeamASpawnOffsets.Add(FVector(0.0f, -2400.0f, 50.0f));	  // Forward 중앙
	TeamASpawnOffsets.Add(FVector(900.0f, -2400.0f, 50.0f));  // Forward 하단
	
	TeamBSpawnOffsets.Empty();
	TeamBSpawnOffsets.Add(FVector(-900.0f, 2400.0f, 50.0f)); 
	TeamBSpawnOffsets.Add(FVector(0.0f, 2400.0f, 50.0f));	
	TeamBSpawnOffsets.Add(FVector(900.0f, 2400.0f, 50.0f));  
}

// ═══════════════════════════════════════════════════════
// OnConstruction — 에디터에서 실시간 미리보기
// ═══════════════════════════════════════════════════════
void ACoreArena::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	RefreshArenaMesh();
	SetupMeshCollision();
	SetupGoalTriggers();
}

// ═══════════════════════════════════════════════════════
// 레벨의 GoalBarrier 자동 수집 + 이벤트 바인딩
// ═══════════════════════════════════════════════════════
void ACoreArena::CollectBarriers()
{
	FoundBarriers.Empty();
	
	TArray<AActor*> foundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACoreBall::StaticClass(), foundActors);
	
	for (AActor* actor : foundActors)
	{
		AGoalBarrier* barrier = Cast<AGoalBarrier>(actor);
		if (barrier)
		{
			FoundBarriers.Add(barrier);
			
			// 파괴 이벤트 등록
			barrier->OnBarrierDestroyed.AddDynamic(this, &ACoreArena::OnBarrierDestroyed);
		}
	}
	
	LOG_GT(TEXT("Found %d GoalBarriers in level"), FoundBarriers.Num());
}

// ═══════════════════════════════════════════════════════
// 배리어 파괴 콜백 → 해당 팀 게이트 체크
// ═══════════════════════════════════════════════════════
void ACoreArena::OnBarrierDestroyed(AGoalBarrier* Barrier)
{
	if (!Barrier || !HasAuthority()) return;
	
	LOG_GT(TEXT("BarrierDestroyed! Team %d"), Barrier->TeamIndex);
	CheckGateStatus(Barrier->TeamIndex);
}

// ═══════════════════════════════════════════════════════
// 해당 팀의 배리어가 전부 파괴되었는지 체크
// ═══════════════════════════════════════════════════════
void ACoreArena::CheckGateStatus(int32 TeamIndex)
{
	bool bAllDestroyed = true;
	
	for (AGoalBarrier* barrier : FoundBarriers)
	{
		if (barrier && barrier->TeamIndex == TeamIndex && !barrier->bIsDestroyed)
		{
			bAllDestroyed = false;
			break;
		}
	}
	
	if (bAllDestroyed)
	{
		LOG_GT(TEXT("전체 배리어 파괴됨. Team %d 게이트 오픈 %.1fs"), TeamIndex, GateOpenDelay);
		
		// 딜레이 후 게이트 오픈
		FTimerHandle& handle = (TeamIndex == 0) ? LeftGateTimerHandle : RightGateTimerHandle;
		FTimerDelegate timerDelegate;
		timerDelegate.BindLambda([this, TeamIndex]() { OpenGate(TeamIndex); });
		GetWorldTimerManager().SetTimer(handle, timerDelegate, GateOpenDelay, false);
	}
}

// ═══════════════════════════════════════════════════════
// 게이트 오픈 (딜레이 후 호출)
// ═══════════════════════════════════════════════════════
void ACoreArena::OpenGate(int32 TeamIndex)
{
	if (TeamIndex == 0)
	{
		bLeftGateOpen = true;
	}
	else
	{
		bRightGateOpen = true;
	}
	
	LOG_GT(TEXT("Team %d 게이트 오픈!"), TeamIndex);
	// TODO: 골 게이트 오픈 연출
}

// ═══════════════════════════════════════════════════════
// 골 오버랩 — 게이트가 오픈되어 있을 때만 득점
// ═══════════════════════════════════════════════════════
void ACoreArena::OnGoalOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 코어인지 체크
	ACoreBall* core = Cast<ACoreBall>(OtherActor);
	if (!core) return;
	
	// 서버에서만 처리
	if (!HasAuthority()) return;
	
	// 어느 쪽 골대인지 판별
	int32 scoringTeam = -1;
	if (OverlappedComp == LeftGoalTrigger && bLeftGateOpen)
	{
		// 왼쪽 골대에 들어감 -> 오른쪽 (팀B) 득점
		scoringTeam = 1;
	}
	else if (OverlappedComp == RightGoalTrigger && bRightGateOpen)
	{
		// 오른쪽 골대에 들어감 -> 왼쪽 (팀A) 득점
		scoringTeam = 0;
	}
	
	if (scoringTeam >= 0)
	{
		LOG_GT(TEXT("GOAL! Team %d scores!"), scoringTeam);
		// GameMode에 골 알림 (기존 GoalZone 로직과 연동)
		// TODO: OSGameMode::OnGoalScored(ScoringTeam) 호출
	}
}

// ═══════════════════════════════════════════════════════
// 라운드 리셋
// ═══════════════════════════════════════════════════════
void ACoreArena::ResetForNewRound()
{
	// 게이트 닫기
	bLeftGateOpen = false;
	bRightGateOpen = false;
	
	// 타이머 캔슬
	GetWorldTimerManager().ClearTimer(LeftGateTimerHandle);
	GetWorldTimerManager().ClearTimer(RightGateTimerHandle);
	
	// 전체 배리어 리셋
	for (AGoalBarrier* barrier : FoundBarriers)
	{
		if (barrier) barrier->ResetBarrier();
	}
	
	LOG_GT(TEXT("라운드 리셋: 전체 배리어 리셋, 게이트 Close"));
}

// ═══════════════════════════════════════════════════════
// 유틸리티
// ═══════════════════════════════════════════════════════
FVector ACoreArena::GetCoreSpawnLocation() const
{
	return GetActorLocation() + CoreSpawnOffset;
}

TArray<FVector> ACoreArena::GetTeamSpawnLocations(int32 TeamIndex) const
{
	TArray<FVector> worldLocations;
	const TArray<FVector>& offsets = (TeamIndex == 0) ? TeamASpawnOffsets : TeamBSpawnOffsets;
	
	for (const FVector& offset : offsets)
	{
		worldLocations.Add(GetActorLocation() + offset);
	}
	
	return worldLocations;
}

bool ACoreArena::IsOutOfBounds(const FVector& Location) const
{
	FVector localPos = Location - GetActorLocation();
	return FMath::Abs(localPos.X) > KnockoutBoundsExtent.X
		|| FMath::Abs(localPos.Y) > KnockoutBoundsExtent.Y
		|| FMath::Abs(localPos.Z) > KnockoutBoundsExtent.Z;
}

#if WITH_EDITOR
void ACoreArena::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	RefreshArenaMesh();
	SetupMeshCollision();
	SetupGoalTriggers();
}
#endif

bool ACoreArena::IsGateOpen(int32 TeamIndex) const
{
	return (TeamIndex == 0) ? bLeftGateOpen : bRightGateOpen;
}

// ═══════════════════════════════════════════════════════
// 아레나 메시 적용
// ═══════════════════════════════════════════════════════
void ACoreArena::RefreshArenaMesh()
{
	if (!ArenaMesh) return;
	
	if (ArenaMeshAsset) ArenaMesh->SetStaticMesh(ArenaMeshAsset);
	if (WallMeshAsset) WallMesh->SetStaticMesh(WallMeshAsset);
	
	ArenaMesh->SetRelativeLocation(MeshOffset);
	ArenaMesh->SetRelativeRotation(MeshRotation);
	ArenaMesh->SetRelativeScale3D(MeshScale);
}

// ═══════════════════════════════════════════════════════
// 메시 콜리전 세팅
// ═══════════════════════════════════════════════════════
void ACoreArena::SetupMeshCollision()
{
	if (!ArenaMesh) return;
	
	// 물리 메테리얼 생성 / 업데이트
	if (!ArenaPhysMat)
	{
		ArenaPhysMat = NewObject<UPhysicalMaterial>(this, TEXT("ArenaPhysMat"));
	}
	ArenaPhysMat->Restitution = WallRestitution;
	ArenaPhysMat->Friction = WallFriction;
	ArenaPhysMat->RestitutionCombineMode = EFrictionCombineMode::Max;
	
	if (bUseMeshCollision)
	{
		// 메시 자체 콜리전 사용 -> 곡선 벽이 그대로 반영
		// aptl 에디터에서 콜리전이 설정되어 있어야 함
		// Simple Collision 또는 Complex as Simple
		ArenaMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		ArenaMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
		ArenaMesh->SetCollisionResponseToAllChannels(ECR_Block);
		// 코어(Overlap 채널)만 Block
		ArenaMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
		
		// 물리 메테리얼 적용
		ArenaMesh->SetPhysMaterialOverride(ArenaPhysMat);
	}
	else
	{
		// 메시는 비주얼만, 콜리전은 별도 볼륨 처리
		ArenaMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

// ═══════════════════════════════════════════════════════
// 골 트리거 위치/크기 업데이트
// ═══════════════════════════════════════════════════════
void ACoreArena::SetupGoalTriggers()
{
	if (LeftGoalTrigger)
	{
		LeftGoalTrigger->SetRelativeLocation(LeftGoalLocation);
		LeftGoalTrigger->SetBoxExtent(GoalExtent);
		LeftGoalTrigger->ComponentTags.Empty();
		LeftGoalTrigger->ComponentTags.Add(FName("Goal_Left"));
	}
	
	if (RightGoalTrigger)
	{
		RightGoalTrigger->SetRelativeLocation(RightGoalLocation);
		RightGoalTrigger->SetBoxExtent(GoalExtent);
		RightGoalTrigger->ComponentTags.Empty();
		RightGoalTrigger->ComponentTags.Add(FName("Goal_Right"));
	}
	
	// 골대 메시
	if (LeftGoalMesh)
	{
		if (LeftGoalMeshAsset) LeftGoalMesh->SetStaticMesh(LeftGoalMeshAsset);
		LeftGoalMesh->SetRelativeLocation(LeftGoalMeshOffset);
		// 좌측 골대는 -90도 회전
		FRotator leftRot = GoalMeshRotation;
		leftRot.Yaw -= 90;
		LeftGoalMesh->SetRelativeRotation(leftRot);
		LeftGoalMesh->SetRelativeScale3D(GoalMeshScale);
	}
	
	if (RightGoalMesh)
	{
		if (RightGoalMeshAsset) RightGoalMesh->SetStaticMesh(RightGoalMeshAsset);
		RightGoalMesh->SetRelativeLocation(RightGoalMeshOffset);
		// 우측 골대는 90도 회전
		FRotator rightRot = GoalMeshRotation;
		rightRot.Yaw += 90;
		RightGoalMesh->SetRelativeRotation(rightRot);
		RightGoalMesh->SetRelativeScale3D(GoalMeshScale);
	}
	
	if (KnockoutBounds)
	{
		KnockoutBounds->SetRelativeLocation(FVector::ZeroVector);
		KnockoutBounds->SetBoxExtent(KnockoutBoundsExtent);
	}
}
