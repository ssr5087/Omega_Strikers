// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ResultWidget.generated.h"

/**
 * 
 */
UCLASS()
class OMEGA_STRIKERS_API UResultWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	class UWidgetSwitcher* WidgetSwitcher;
	
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* txt_BlueScore;
	
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* txt_RedScore;
	
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* txt_Score_B1;
	
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* txt_Score_B2;
	
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* txt_Score_B3;
	
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* txt_Score_R1;
	
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* txt_Score_R2;
	
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* txt_Score_R3;
	
	void SetScore(int32 blue, int32 red, int32 B1, int32 B2, int32 B3, int32 R1,int32 R2, int32 R3);
};
