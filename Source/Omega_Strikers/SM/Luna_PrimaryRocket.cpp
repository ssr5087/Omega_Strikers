// Fill out your copyright notice in the Description page of Project Settings.


#include "Luna_PrimaryRocket.h"

#include "Components/BoxComponent.h"


// Sets default values
ALuna_PrimaryRocket::ALuna_PrimaryRocket()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	BoxComp = CreateDefaultSubobject<UBoxComponent>("BoxComp");
	SetRootComponent(BoxComp);
	BoxComp->SetBoxExtent(FVector(150.0f, 55.0f, 55.0f));
	
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>("RocketMesh");
	ConstructorHelpers::FObjectFinder<UStaticMesh> TempMesh(TEXT("/Script/Engine.StaticMesh'/Game/Resource/Luna/Luna_Rocket.Luna_Rocket'"));
	if (TempMesh.Succeeded())
	{
		RocketMesh->SetStaticMesh(TempMesh.Object);
	}
	RocketMesh->SetupAttachment(BoxComp);
	RocketMesh->SetRelativeLocationAndRotation(FVector(10.0f, 0.0f, 0.0f), FRotator(-90.0f, 0.0f, 0.0f));
}

// Called when the game starts or when spawned
void ALuna_PrimaryRocket::BeginPlay()
{
	Super::BeginPlay();
	
	FTimerHandle SpeedChanger;
	GetWorld()->GetTimerManager().SetTimer(SpeedChanger, [this]()->void {Speed = 5000.f;}, 0.5f, false);
}

// Called every frame
void ALuna_PrimaryRocket::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	SetActorLocation(GetActorLocation() + GetActorForwardVector() * DeltaTime * Speed);
}

