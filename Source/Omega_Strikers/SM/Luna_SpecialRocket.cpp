// Fill out your copyright notice in the Description page of Project Settings.


#include "Luna_SpecialRocket.h"

#include "PlayerBase.h"
#include "Components/BoxComponent.h"
#include "Core/CoreBall.h"
#include "Kismet/GameplayStatics.h"
#include "Omega_Strikers/Omega_Strikers.h"


// Sets default values
ALuna_SpecialRocket::ALuna_SpecialRocket()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	BoxComp = CreateDefaultSubobject<UBoxComponent>("BoxComp");
	SetRootComponent(BoxComp);
	BoxComp->SetBoxExtent(FVector(100.0f, 100.0f, 100.0f));
	BoxComp->OnComponentBeginOverlap.AddDynamic(this, &ALuna_SpecialRocket::OnSpecialRocketOverlap);
	
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>("RocketMesh");
	ConstructorHelpers::FObjectFinder<UStaticMesh> TempMesh(TEXT("/Script/Engine.StaticMesh'/Game/Resource/Luna/Luna_Rocket.Luna_Rocket'"));
	if (TempMesh.Succeeded())
	{
		RocketMesh->SetStaticMesh(TempMesh.Object);
	}
	RocketMesh->SetupAttachment(BoxComp);
	RocketMesh->SetRelativeLocationAndRotation(FVector(-150.0f, 0.0f, 0.0f), FRotator(-90.0f, 0.0f, 0.0f));
	RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Called when the game starts or when spawned
void ALuna_SpecialRocket::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ALuna_SpecialRocket::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	SetActorLocation(GetActorLocation() + GetActorForwardVector() * DeltaTime * 5000);
}

void ALuna_SpecialRocket::InitRocket(AActor* InOwnerActor)
{
	OwnerActorRef = InOwnerActor;
	
	// 데이터 적용 필요
	SpecialNearImpactData.CoreKnockbackPower = 3000.f;
	SpecialNearImpactData.PlayerKnockbackPower = 300.f;
	SpecialNearImpactData.PlayerDamage = 100.f;
	SpecialFarImpactData.CoreKnockbackPower = 5000.f;
	SpecialFarImpactData.PlayerKnockbackPower = 500.f;
	SpecialFarImpactData.PlayerDamage = 200.f;
}

void ALuna_SpecialRocket::OnSpecialRocketOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 충돌 지점의 월드 위치
	FVector2D fallLoc = FVector2D(GetActorLocation().X, GetActorLocation().Y);
	
	if (OtherActor && OtherActor->GetName().Contains("Arena"))
	{
		TArray<AActor*> TempPlayers;
		
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerBase::StaticClass(), TempPlayers);
		AActor* Core = UGameplayStatics::GetActorOfClass(GetWorld(), ACoreBall::StaticClass());
		if (Core)
		{
			TempPlayers.Add(Core);
		}
		
		for (auto actor : TempPlayers)
		{
			FVector2D actLoc = FVector2D(actor->GetActorLocation().X, actor->GetActorLocation().Y);
			if (FVector2D::Distance(fallLoc, actLoc) <= 500.f)
			{
				SpecialNearImpactData.Direction = FVector2D(actor->GetActorLocation().X - fallLoc.X, actor->GetActorLocation().Y - fallLoc.Y).GetSafeNormal();
				IOSImpactReceiver::Execute_ReceiveImpact(actor, SpecialNearImpactData, OwnerActorRef);
				continue;
			}
			if (FVector2D::Distance(fallLoc, actLoc) <= 1000.f)
			{
				SpecialFarImpactData.Direction = FVector2D(actor->GetActorLocation().X - fallLoc.X, actor->GetActorLocation().Y - fallLoc.Y).GetSafeNormal();
				IOSImpactReceiver::Execute_ReceiveImpact(actor, SpecialFarImpactData, OwnerActorRef);
			}
		}
		Destroy();
	}
}

