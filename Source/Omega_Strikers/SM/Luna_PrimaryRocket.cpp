// Fill out your copyright notice in the Description page of Project Settings.


#include "Luna_PrimaryRocket.h"

#include "PlayerBase.h"
#include "Components/BoxComponent.h"
#include "Core/CoreBall.h"


// Sets default values
ALuna_PrimaryRocket::ALuna_PrimaryRocket()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	BoxComp = CreateDefaultSubobject<UBoxComponent>("BoxComp");
	SetRootComponent(BoxComp);
	BoxComp->SetBoxExtent(FVector(150.0f, 55.0f, 55.0f));
	BoxComp->OnComponentBeginOverlap.AddDynamic(this, &ALuna_PrimaryRocket::OnRocketOverlap);
	
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>("RocketMesh");
	ConstructorHelpers::FObjectFinder<UStaticMesh> TempMesh(TEXT("/Script/Engine.StaticMesh'/Game/Resource/Luna/Luna_Rocket.Luna_Rocket'"));
	if (TempMesh.Succeeded())
	{
		RocketMesh->SetStaticMesh(TempMesh.Object);
	}
	RocketMesh->SetupAttachment(BoxComp);
	RocketMesh->SetRelativeLocationAndRotation(FVector(10.0f, 0.0f, 0.0f), FRotator(-90.0f, 0.0f, 0.0f));
	RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Called when the game starts or when spawned
void ALuna_PrimaryRocket::BeginPlay()
{
	Super::BeginPlay();
	
	// 데이터 적용 필요
	ImpactData.Direction = FVector2D(GetActorForwardVector().X, GetActorForwardVector().Y).GetSafeNormal();
	ImpactData.CoreKnockbackPower = 3390.f;
	ImpactData.PlayerKnockbackPower = 1500.f;
	ImpactData.PlayerDamage = 100.0f;
	
	FTimerHandle SpeedChanger;
	GetWorld()->GetTimerManager().SetTimer(SpeedChanger, [this]()->void {Speed = 5000.f;}, 0.5f, false);
}

// Called every frame
void ALuna_PrimaryRocket::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	SetActorLocation(GetActorLocation() + GetActorForwardVector() * DeltaTime * Speed);
}

void ALuna_PrimaryRocket::InitRocket(float Owner_Power, AActor* InOwnerActor, EOSTeam InTeamSide)
{
	Luna_Power = Owner_Power;
	OwnerActorRef = InOwnerActor;
	TeamSide = InTeamSide;
}

// 충돌 시 데미지 전달해주는 로직 넣기
void ALuna_PrimaryRocket::OnRocketOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// OtherActor가 존재하고 그 놈이 인터페이스 구현했으면
	if (OtherActor && OtherActor->Implements<UOSImpactReceiver>())
	{
		// 그 대상에 대해 인터페이스 함수를 실행해라
		IOSImpactReceiver::Execute_ReceiveImpact(OtherActor, ImpactData, GetOwner());
	}
	
	this->Destroy();
}

