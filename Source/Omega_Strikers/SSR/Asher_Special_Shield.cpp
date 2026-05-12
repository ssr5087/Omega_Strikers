// Fill out your copyright notice in the Description page of Project Settings.


#include "Asher_Special_Shield.h"

#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Omega_Strikers/SM/OSImpactReceiver.h"
#include "PlayerBase.h"
#include "Omega_Strikers/Omega_Strikers.h"


// Sets default values
AAsher_Special_Shield::AAsher_Special_Shield()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	// Root
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	// Collision 
	CollisionComp = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionComp"));
	CollisionComp->SetupAttachment(Root);

	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComp->SetGenerateOverlapEvents(true);
	CollisionComp->SetCollisionProfileName(TEXT("OverlapAll"));
	
	CollisionComp->SetRelativeLocation(FVector(80.000000,0.000000,0.000000));
	CollisionComp->SetRelativeRotation(FRotator(0.000000,0.000000,0.000000));
	CollisionComp->SetRelativeScale3D(FVector(6.500000,12.250000,4.250000));
	
	// 메쉬 생성
	Mesh1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh1"));
	Mesh2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh2"));

	// Attach
	Mesh1->SetupAttachment(RootComponent);
	Mesh2->SetupAttachment(RootComponent);

	// 메쉬 로드
	static ConstructorHelpers::FObjectFinder<UStaticMesh> TempMesh(TEXT("/Script/Engine.StaticMesh'/Game/Resource/Asher/Asher_Shield.Asher_Shield'"));

	if (TempMesh.Succeeded())
	{
		Mesh1->SetStaticMesh(TempMesh.Object);
		Mesh2->SetStaticMesh(TempMesh.Object);
	}

	// Mesh1 설정
	Mesh1->SetRelativeLocation(FVector(91.281336,-208.576372,-0.000000));
	Mesh1->SetRelativeRotation(FRotator(90.000000,-140.000000,0.000000));
	Mesh1->SetRelativeScale3D(FVector(3.f));
	Mesh1->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Mesh2 설정
	Mesh2->SetRelativeLocation(FVector(87.583631,210.841334,-0.000000));
	Mesh2->SetRelativeRotation(FRotator(-90.000000,-40.000000,0.000000));
	Mesh2->SetRelativeScale3D(FVector(3.f));
	Mesh2->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
}

// Called when the game starts or when spawned
void AAsher_Special_Shield::BeginPlay()
{
	Super::BeginPlay();
	
	// 🔥 Owner 캐싱 (핵심)
	OwnerPlayer = Cast<APlayerBase>(GetOwner());

	if (HasAuthority())
	{
		// 첫 틱이 늦게 시작되면 "한 번만 맞는 것처럼" 보일 수 있어서 즉시 1회 적용
		ApplyDamage();

		GetWorld()->GetTimerManager().SetTimer(
			DamageTimer,
			this,
			&AAsher_Special_Shield::ApplyDamage,
			DamageInterval,
			true
		);
		
		// 1.5초 후 삭제
		GetWorld()->GetTimerManager().SetTimer(
			LifeTimer,
			[this]()
			{
				Destroy();
			},
			2.0f,
			false
		);
	}
}

// Called every frame
void AAsher_Special_Shield::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (HasAuthority())
	{
		// 방패 이동
		AddActorWorldOffset(MoveDirection* MoveSpeed * DeltaTime, true);
	}
}

void AAsher_Special_Shield::Init(const FVector& InDirection, const FRotator& InRotation)
{
	MoveDirection = FVector(InDirection.X, InDirection.Y, 0.f).GetSafeNormal();
	SetActorRotation(InRotation);
}

bool AAsher_Special_Shield::IsCenter(AActor* Target)
{
	float Dist = FVector::Dist(Target->GetActorLocation(), GetActorLocation());
	return Dist <= CenterRadius;
}

void AAsher_Special_Shield::ApplyDamage()
{
	LOG_SR_W(TEXT("ApplyDamage Called"));
	if (!HasAuthority())
	{
		return;
	}

	if (!OwnerPlayer)
		return;
	
	HitActors.Empty(); 
	CollisionComp->UpdateOverlaps();
	
	TArray<AActor*> OverlappingActors;
	CollisionComp->GetOverlappingActors(OverlappingActors);
	if (OverlappingActors.Num() == 0)
	{
		// overlap 갱신이 지연되는 경우를 대비한 보강 판정
		GetOverlappingActors(OverlappingActors);
	}
	
	for (auto Target : OverlappingActors)
	{
		if (!Target || Target == GetOwner())
			continue;
		if (!Target->Implements<UOSImpactReceiver>())
			continue;
		if (HitActors.Contains(Target))
			continue;
		
		LOG_SR_W(TEXT("Hit : %s"), *Target->GetName());
		FName RowName = IsCenter(Target)
			? TEXT("Asher_Special_Center")
			: TEXT("Asher_Special_Side");
		
		FCharacterSkill* Skill = OwnerPlayer->GetSkillData(RowName);
		if (!Skill)
			continue;
		
		// 🔥 CSV 기반 데미지 계산
		FOSImpactData Data = OwnerPlayer->MakeImpactData(*Skill);
		
		// 추가
		FVector2D Dir = FVector2D(
	Target->GetActorLocation().X - GetActorLocation().X,
	Target->GetActorLocation().Y - GetActorLocation().Y
		).GetSafeNormal();

		Data.Direction = Dir;
		
		IOSImpactReceiver::Execute_ReceiveImpact(Target, Data, this);
		
		HitActors.Add(Target);
		
	}
	
}

