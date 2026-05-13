// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ScoreBoardWidget.generated.h"

/**
 * 
 */
UCLASS()
class OMEGA_STRIKERS_API UScoreBoardWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* txt_ScoreBlue;
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* txt_ScoreRed;
	
	void SetScore(int32 Blue, int32 Red);
};
