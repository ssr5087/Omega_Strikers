// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "SessionItemWidget.generated.h"

/**
 * 
 */
UCLASS()
class OMEGA_STRIKERS_API USessionItemWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void Setup(int32 InIndex);  // ⭐ 이거 있어야 함

public:
	virtual bool Initialize() override;

	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton;

	int32 SessionIndex;

	UFUNCTION()
	void OnClicked();
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* RoomNameText;
};
