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
		IndicatorMesh->SetRelativeScale3D(IndicatorMeshScale);
	}
}

void ASkillIndicatorBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (IndicatorMesh)
	{
		IndicatorMesh->SetRelativeLocation(IndicatorMeshLocation);
		IndicatorMesh->SetRelativeRotation(IndicatorMeshRotation);
		IndicatorMesh->SetRelativeScale3D(IndicatorMeshScale);
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

	// =========================
	// Directional
	// =========================
	if (IndicatorMode == EIndicatorMode::Directional)
	{
		SetActorLocation(
			OwnerLocation + IndicatorWorldOffset
		);

		SetActorRotation(
			FRotator(
				0.f,
				AimDirection.Rotation().Yaw + IndicatorYawOffset,
				0.f
			)
		);
	}

	// =========================
	// TargetLocation
	// =========================
	else if (IndicatorMode == EIndicatorMode::TargetLocation)
	{
		// 최대 사거리 제한
		MouseOffset = MouseOffset.GetClampedToMaxSize(Range);

		FVector TargetLocation =
			OwnerLocation + MouseOffset;

		TargetLocation.Z += IndicatorWorldOffset.Z;

		SetActorLocation(TargetLocation);

		// 회전 필요하면 유지
		SetActorRotation(
			FRotator(
				0.f,
				AimDirection.Rotation().Yaw,
				0.f
			)
		);
	}
}

void ASkillIndicatorBase::SetIndicatorRange(float NewRange)
{
	Range = FMath::Max(0.f, NewRange);

	float ScaleValue = Range / RangeScaleBase;

	IndicatorMesh->SetRelativeScale3D(
		FVector(
			ScaleValue,
			ScaleValue,
			1.f
		)
	);
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
