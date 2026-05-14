// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GoalWidget.generated.h"

/**
 * 
 */
UCLASS()
class OMEGA_STRIKERS_API UGoalWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta=(BindWidget))
	class UImage* Image_Luna;
	
	UPROPERTY(meta=(BindWidget))
	class UImage* Image_Asher;
	
	UPROPERTY(meta=(BindWidget))
	class UImage* Image_Aimi;
	
	UPROPERTY(Transient, meta=(BindWidgetAnim))
	class UWidgetAnimation* Goal;
	
	void PlayGoalAnimation(int ScorerIndex);
};
