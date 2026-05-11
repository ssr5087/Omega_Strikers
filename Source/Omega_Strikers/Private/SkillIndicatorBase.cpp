// Fill out your copyright notice in the Description page of Project Settings.


#include "SkillIndicatorBase.h"

#include "Components/StaticMeshComponent.h"
#include "PlayerBase.h"


// Sets default values
ASkillIndicatorBase::ASkillIndicatorBase()
{
	PrimaryActorTick.bCanEverTick = true;

	IndicatorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("IndicatorMesh"));
	RootComponent = IndicatorMesh;

	// 충돌 제거
	IndicatorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	IndicatorMesh->SetGenerateOverlapEvents(false);
	IndicatorMesh->SetCastShadow(false);
	IndicatorMesh->SetRelativeScale3D(IndicatorMeshScale);
	IndicatorMesh->SetRelativeLocation(IndicatorMeshLocation);
}

// Called when the game starts or when spawned
void ASkillIndicatorBase::BeginPlay()
{
	Super::BeginPlay();

	if (IndicatorMesh)
	{
		IndicatorMesh->SetRelativeScale3D(IndicatorMeshScale);
		IndicatorMesh->SetRelativeLocation(IndicatorMeshLocation);
	}
}

void ASkillIndicatorBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (IndicatorMesh)
	{
		IndicatorMesh->SetRelativeScale3D(IndicatorMeshScale);
		IndicatorMesh->SetRelativeLocation(IndicatorMeshLocation);
	}
}

// Called every frame
void ASkillIndicatorBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ASkillIndicatorBase::UpdateIndicator(class APlayerBase* OwnerPlayer)
{
	if (!OwnerPlayer)
	{
		return;
	}

	const FVector OwnerLocation = OwnerPlayer->GetActorLocation();
	FVector MouseOffset = OwnerPlayer->MouseCursorLoc - OwnerLocation;
	MouseOffset.Z = 0.f;

	FVector AimDirection = MouseOffset.GetSafeNormal();
	if (AimDirection.IsNearlyZero())
	{
		AimDirection = OwnerPlayer->GetActorForwardVector().GetSafeNormal2D();
	}

	FVector TargetLocation = OwnerLocation + FVector(0.f, 0.f, HeightOffset);

	SetActorLocation(TargetLocation);
	SetActorRotation(FRotator(0.f, AimDirection.Rotation().Yaw, 0.f));
}

void ASkillIndicatorBase::SetIndicatorRange(float NewRange)
{
	Range = FMath::Max(0.f, NewRange);
}

void ASkillIndicatorBase::SetIndicatorMesh(
	UStaticMesh* NewMesh,
	const FVector& RelativeScale,
	const FVector& RelativeLocation,
	const FRotator& RelativeRotation)
{
	if (!IndicatorMesh)
	{
		return;
	}

	IndicatorMesh->SetStaticMesh(NewMesh);
	IndicatorMesh->SetRelativeScale3D(RelativeScale);
	IndicatorMesh->SetRelativeLocation(RelativeLocation);
	IndicatorMesh->SetRelativeRotation(RelativeRotation);
	IndicatorMeshScale = RelativeScale;
	IndicatorMeshLocation = RelativeLocation;
}

