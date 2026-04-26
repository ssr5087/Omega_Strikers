// Fill out your copyright notice in the Description page of Project Settings.


#include "Asher_Special_Projectile.h"

#include "Asher_Special_Shield.h"
#include "Components/BoxComponent.h"
#include "Omega_Strikers/SM/OSImpactReceiver.h"


// Sets default values
AAsher_Special_Projectile::AAsher_Special_Projectile()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
	
	CollisionComp = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionComp"));
	CollisionComp->SetupAttachment(Root);
	
	// 메쉬 생성
	Mesh1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh1"));
	Mesh2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh2"));
	
	Mesh1->SetupAttachment(Root);
	Mesh2->SetupAttachment(Root);
	
	// 메쉬 로드
	static ConstructorHelpers::FObjectFinder<UStaticMesh> TempMesh(TEXT("/Script/Engine.StaticMesh'/Game/Resource/Asher/Asher_Shield.Asher_Shield'"));

	if (TempMesh.Succeeded())
	{
		Mesh1->SetStaticMesh(TempMesh.Object);
		Mesh2->SetStaticMesh(TempMesh.Object);
	}
	
	
}

// Called when the game starts or when spawned
void AAsher_Special_Projectile::BeginPlay()
{
	Super::BeginPlay();
	
	CollisionComp->OnComponentBeginOverlap.AddDynamic(
		this,
		&AAsher_Special_Projectile::OnHitOverlap
	);
	
	if (ShieldClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShieldClass OK"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ShieldClass NULL at BeginPlay"));
	}
}

// Called every frame
void AAsher_Special_Projectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	AddActorWorldOffset(MoveDirection * Speed * DeltaTime);
}

void AAsher_Special_Projectile::Init(const FVector& Dir, EOSTeam InTeam)
{
	MoveDirection = FVector(Dir.X, Dir.Y, 0.f).GetSafeNormal();
	OwnerTeam = InTeam;
	
}

void AAsher_Special_Projectile::OnHitOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	
	if (!OtherActor || OtherActor == GetOwner())
	{
		auto name = OtherActor->GetName();
		//GEngine->AddOnScreenDebugMessage(1,1,FColor::Magenta,name);
		return;
	}
	
	if (!OtherActor->Implements<UOSImpactReceiver>())
	{
		
		//UE_LOG(LogTemp, Warning, );
		auto name = OtherActor->GetName();
		//GEngine->AddOnScreenDebugMessage(2,1,FColor::Magenta,name);
		return;
	}
	
	FOSImpactData Data;
	Data.TeamSide = OwnerTeam;
	Data.Direction = FVector2D(MoveDirection.X, MoveDirection.Y);
	Data.CoreKnockbackPower = 1.f;
	
	// 맞아도 되는 대상인지 체크
	if (IOSImpactReceiver::Execute_ReceiveImpact(OtherActor, Data, GetOwner()))
	{
		const FRotator ProjectileRotation = GetActorRotation();
		
		// Shield 생성
		if (ShieldClass)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = GetOwner();

			AAsher_Special_Shield* Shield = GetWorld()->SpawnActor<AAsher_Special_Shield>(
				ShieldClass,
				GetActorLocation(),
				ProjectileRotation,
				SpawnParams
			);
			if (Shield)
			{
				Shield->Init(MoveDirection, ProjectileRotation);
			}
			
		}
		Destroy();
	}
	
	
}
