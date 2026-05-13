// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HPStatusWidget.generated.h"

enum class EOSTeam : uint8;
/**
 * 
 */
UCLASS()
class OMEGA_STRIKERS_API UHPStatusWidget : public UUserWidget
{
	GENERATED_BODY()

	virtual void NativeConstruct() override;
	
public:
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	class UImage* img_StaggerBar_Teamside;
	
	UPROPERTY()
	UMaterialInstanceDynamic* CurrentStaggerBar;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UMaterialInterface* BlueTeam;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UMaterialInterface* RedTeam;
	
	void SetTeamSide(EOSTeam teamSide);
	void SetStaggerPercent(float Percent);
	
public:
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	class UTextBlock* txt_level;
	
	void SetLevel(int32 level);
};
