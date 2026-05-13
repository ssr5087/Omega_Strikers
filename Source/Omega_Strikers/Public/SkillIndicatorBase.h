// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SkillIndicatorBase.generated.h"

class USceneComponent;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class EIndicatorMode : uint8
{
	Directional		UMETA(DisplayName = "Directional"),
	TargetLocation	UMETA(DisplayName = "Target Location")
};

UCLASS()
class OMEGA_STRIKERS_API ASkillIndicatorBase : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASkillIndicatorBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	// 루트 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Indicator")
	USceneComponent* SceneRoot;
	
	// 모드 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Indicator")
	EIndicatorMode IndicatorMode = EIndicatorMode::Directional;

	// 범위 표시용 메시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Indicator")
	UStaticMeshComponent* IndicatorMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Indicator")
	FVector IndicatorMeshScale = FVector(1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Indicator")
	FVector IndicatorMeshLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Indicator")
	FRotator IndicatorMeshRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Indicator")
	FVector IndicatorWorldOffset = FVector(0.f, 0.f, 100.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Indicator")
	float IndicatorYawOffset = 0.f;
	
	// 업데이트
	virtual void UpdateIndicator(class APlayerBase* OwnerPlayer);

	void SetIndicatorRange(float NewRange);
	void SetIndicatorMesh(
		class UStaticMesh* NewMesh,
		const FVector& RelativeScale = FVector(1.f),
		const FVector& RelativeLocation = FVector::ZeroVector,
		const FRotator& RelativeRotation = FRotator::ZeroRotator);
	
	// 사거리
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Indicator")
	float Range = 500;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Indicator")
	float HeightOffset = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Indicator")
	float RangeScaleBase = 500.f;
};


