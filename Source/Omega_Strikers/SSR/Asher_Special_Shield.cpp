// Fill out your copyright notice in the Description page of Project Settings.


#include "Asher_Special_Shield.h"

#include "Kismet/GameplayStatics.h"
#include "Omega_Strikers/SM/OSImpactReceiver.h"


// Sets default values
AAsher_Special_Shield::AAsher_Special_Shield()
{
	// 루트 생성
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

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
	Mesh1->SetRelativeLocation(FVector(68.3f, -227.86f, 0.f));
	Mesh1->SetRelativeRotation(FRotator(90.f, 40.f, 180.f));
	Mesh1->SetRelativeScale3D(FVector(3.f));
	Mesh1->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Mesh2 설정
	Mesh2->SetRelativeLocation(FVector(68.3f, 187.86f, 0.f));
	Mesh2->SetRelativeRotation(FRotator(270.f, -40.f, 0.f));
	Mesh2->SetRelativeScale3D(FVector(3.f));
	Mesh2->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Called when the game starts or when spawned
void AAsher_Special_Shield::BeginPlay()
{
	Super::BeginPlay();
	
	GetWorld()->GetTimerManager().SetTimer(
		DamageTimer,
		this,
		&AAsher_Special_Shield::ApplyDamage,
		DamageInterval,
		true
	);
}

// Called every frame
void AAsher_Special_Shield::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// 방패 이동
	AddActorWorldOffset(MoveDirection* MoveSpeed * DeltaTime);
}

void AAsher_Special_Shield::Init(const FVector& InDirection)
{
	MoveDirection = FVector(InDirection.X, InDirection.Y, 0.f).GetSafeNormal();
}

bool AAsher_Special_Shield::IsCenter(AActor* Target)
{
	float Dist = FVector::Dist(Target->GetActorLocation(), GetActorLocation());
	return Dist < CenterRadius;
}

void AAsher_Special_Shield::ApplyDamage()
{
	HitActors.Empty(); 
	
	TArray<AActor*> OverlappingActors;
	GetOverlappingActors(OverlappingActors);
	
	for (auto Target : OverlappingActors)
	{
		if (!Target || Target == GetOwner())
			continue;
		if (!Target->Implements<UOSImpactReceiver>())
			continue;
		if (HitActors.Contains(Target))
			continue;
		
		// 방향 (중심 -> 대상), 수정 필요할듯
		FVector2D Dir = FVector2D(
			Target->GetActorLocation().X - GetActorLocation().X,
			Target->GetActorLocation().Y - GetActorLocation().Y
		).GetSafeNormal();
		
		FOSImpactData Data;
		// 방향 수정 필요할듯
		Data.Direction = Dir;
		// 중앙과 사이드 데미지 
		Data.PlayerDamage = IsCenter(Target) ? CenterDamage : EdgeDamage;
		
		Data.PlayerKnockbackPower = PlayerKnockback;
		Data.CoreKnockbackPower = CoreKnockback;
		
		IOSImpactReceiver::Execute_ReceiveImpact(Target, Data, this);
		
		HitActors.Add(Target);
		
	}
	
}

