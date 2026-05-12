// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "OSPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class OMEGA_STRIKERS_API AOSPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;
	
	bool GetMousePointOnArenaPlane(FVector& OutPoint) const;

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<class UPlayerHUDWidget> PlayerHUDWidgetClass;

	UPROPERTY()
	class UPlayerHUDWidget* PlayerHUDWidget;
	
	UPlayerHUDWidget* GetPlayerHUDWidget() const;
	
	FTimerHandle RegisterTimer;
	void RegisterExistingPlayersToHUD();
};
