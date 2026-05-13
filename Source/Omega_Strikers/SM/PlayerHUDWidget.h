// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHUDWidget.generated.h"

/**
 * 
 */
UCLASS()
class OMEGA_STRIKERS_API UPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
public:
	UPROPERTY(meta=(BindWidget))
	class UCanvasPanel* Canvas_Whole;
	
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<class UHPStatusWidget> HPBar;
	
	UPROPERTY()
	TMap<class APlayerBase*, class UHPStatusWidget*> HPWidgetMap;
	
	void RegisterPlayer(class APlayerBase* TargetPlayer);
	void UnRegisterPlayer(class APlayerBase* TargetPlayer);
};
