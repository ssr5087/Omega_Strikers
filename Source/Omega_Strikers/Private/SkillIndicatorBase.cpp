// Fill out your copyright notice in the Description page of Project Settings.


#include "SkillIndicatorBase.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "PlayerBase.h"


// Sets default values
ASkillIndicatorBase::ASkillIndicatorBase()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	IndicatorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("IndicatorMesh"));
	IndicatorMesh->SetupAttachment(SceneRoot);

	// 충돌 제거
	IndicatorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	IndicatorMesh->SetGenerateOverlapEvents(false);
	IndicatorMesh->SetCastShadow(false);
	IndicatorMesh->SetRelativeLocation(IndicatorMeshLocation);
	IndicatorMesh->SetRelativeRotation(IndicatorMeshRotation);
}

// Called when the game starts or when spawned
void ASkillIndicatorBase::BeginPlay()
{
	Super::BeginPlay();

	if (IndicatorMesh)
	{
		IndicatorMesh->SetRelativeLocation(IndicatorMeshLocation);
		IndicatorMesh->SetRelativeRotation(IndicatorMeshRotation);
	}
}

void ASkillIndicatorBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (IndicatorMesh)
	{
		IndicatorMesh->SetRelativeLocation(IndicatorMeshLocation);
		IndicatorMesh->SetRelativeRotation(IndicatorMeshRotation);
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
	const FVector MouseLocation = OwnerPlayer->MouseCursorLoc;
	FVector MouseOffset = MouseLocation - OwnerLocation;
	MouseOffset.Z = 0.f;

	FVector AimDirection = MouseOffset.GetSafeNormal();
	if (AimDirection.IsNearlyZero())
	{
		AimDirection = OwnerPlayer->GetActorForwardVector().GetSafeNormal2D();
	}

	const FVector TargetLocation = OwnerLocation + IndicatorWorldOffset;
	const FRotator TargetRotation = FRotator(
		0,
		AimDirection.Rotation().Yaw + IndicatorYawOffset,
		0.f
	);

	SetActorLocation(TargetLocation);
	SetActorRotation(TargetRotation);
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
	IndicatorMeshRotation = RelativeRotation;
}
