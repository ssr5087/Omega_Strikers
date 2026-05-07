// Fill out your copyright notice in the Description page of Project Settings.


#include "EXPOrb.h"

#include "EXPComponent.h"
#include "PlayerBase.h"
#include "EXPSpawnPoint.h"
#include "Components/SphereComponent.h"
#include "Engine/OverlapResult.h"
#include "Kismet/GameplayStatics.h"
#include "Omega_Strikers/Omega_Strikers.h"


// Sets default values
AEXPOrb::AEXPOrb()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	// 충돌
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
	RootComponent = CollisionComp;
	
	CollisionComp->SetSphereRadius(100.f);
	ConfigureCollision();

	// 메쉬
	// Mesh->CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	// Mesh->SetupAttachment(RootComponent);
	
	// 이벤트 바인딩
	CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &AEXPOrb::OnOverlapBegin);
	
	// 네트워크
	SetReplicates(true);
}

// Called when the game starts or when spawned
void AEXPOrb::BeginPlay()
{
	Super::BeginPlay();

	ConfigureCollision();
	CollisionComp->UpdateOverlaps();

	if (!SpawnPoint)
	{
		TArray<AActor*> FoundSpawnPoints;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEXPSpawnPoint::StaticClass(), FoundSpawnPoints);

		for (AActor* Actor : FoundSpawnPoints)
		{
			AEXPSpawnPoint* Candidate = Cast<AEXPSpawnPoint>(Actor);
			if (Candidate && Candidate->GetActorLocation().Equals(GetActorLocation(), 1.f))
			{
				SpawnPoint = Candidate;
				SpawnPoint->RegisterOrb(this);
				break;
			}
		}
	}
}

void AEXPOrb::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!HasAuthority())
	{
		return;
	}

	PollNearbyPlayers();
}

void AEXPOrb::SetSpawnPoint(AEXPSpawnPoint* InSpawnPoint)
{
	SpawnPoint = InSpawnPoint;

	if (SpawnPoint)
	{
		SpawnPoint->RegisterOrb(this);
	}
}

void AEXPOrb::ConfigureCollision()
{
	if (!CollisionComp)
	{
		return;
	}

	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComp->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionComp->SetGenerateOverlapEvents(true);
}

void AEXPOrb::TryCollect(APlayerBase* Player)
{
	LOG_SR(TEXT("TryCollect 진입"));
	if (!HasAuthority() || !Player)
	{
		LOG_SR_W(TEXT("%s"), Player ? TEXT("HasAuthority is false") : TEXT("Player is NULL"));
		return;
	}

	LOG_SR_W(TEXT("경험치 오브와 충돌"));

	if (Player->EXPComp)
	{
		Player->EXPComp->AddEXP(EXPAmount);
		LOG_SR_W(TEXT("경험치 획득"));
	}

	if (SpawnPoint)
	{
		SpawnPoint->ClearOrb(this);
	}

	Destroy();
}

void AEXPOrb::PollNearbyPlayers()
{
	if (!GetWorld())
	{
		return;
	}

	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(EXPOrbPoll), false, this);
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

	const bool bHasOverlap = GetWorld()->OverlapMultiByObjectType(
		Overlaps,
		GetActorLocation(),
		FQuat::Identity,
		ObjectParams,
		FCollisionShape::MakeSphere(CollisionComp ? CollisionComp->GetScaledSphereRadius() : 100.f),
		QueryParams
	);

	if (!bHasOverlap)
	{
		return;
	}

	for (const FOverlapResult& Overlap : Overlaps)
	{
		APlayerBase* Player = Cast<APlayerBase>(Overlap.GetActor());
		if (!Player)
		{
			continue;
		}

		TryCollect(Player);
		return;
	}
}

void AEXPOrb::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 서버에서만 관리
	if (!HasAuthority())
		return;
	
	if (!OtherActor || OtherActor == this)
		return;

	APlayerBase* player = Cast<APlayerBase>(OtherActor);
	if (!player)
		return;

	TryCollect(player);
}

