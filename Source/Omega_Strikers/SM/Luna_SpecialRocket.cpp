// Fill out your copyright notice in the Description page of Project Settings.


#include "Luna_SpecialRocket.h"

#include "Components/BoxComponent.h"


// Sets default values
ALuna_SpecialRocket::ALuna_SpecialRocket()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	BoxComp = CreateDefaultSubobject<UBoxComponent>("BoxComp");
	SetRootComponent(BoxComp);
	BoxComp->SetBoxExtent(FVector(150.0f, 70.0f, 70.0f));
	BoxComp->OnComponentBeginOverlap.AddDynamic(this, &ALuna_SpecialRocket::OnSpecialRocketOverlap);
	
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
void ALuna_SpecialRocket::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ALuna_SpecialRocket::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ALuna_SpecialRocket::OnSpecialRocketOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	
}

