// Fill out your copyright notice in the Description page of Project Settings.


#include "TempCore.h"


// Sets default values
ATempCore::ATempCore()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ATempCore::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATempCore::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	Speed = FMath::Max(Speed - 1, 0);
	SetActorLocation(GetActorLocation() + Distance * Speed);
}

// 충격 받은 방향으로 이동하는 초기화만 진행. 벽에 튕기는 건 추후에 만들기
bool ATempCore::ReceiveImpact_Implementation(const FOSImpactData& ImpactData, AActor* InstigatorActor)
{
	if (ImpactData.CoreKnockbackPower > 0)
	{
		Speed = ImpactData.CoreKnockbackPower / 10;
		Distance = FVector(ImpactData.Direction.X, ImpactData.Direction.Y, 0);
		return true;
	}
	return false;
}