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
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<class UScoreBoardWidget> ScoreBoardWidgetClass;
	
	UPROPERTY()
	class UScoreBoardWidget* ScoreBoardWidget;
	
	int32 BlueScore = 0;
	int32 RedScore = 0;
	
	FTimerHandle AddTimer;
	void AddScoreBoard();
	UFUNCTION()
	void SetScoreBoard(int32 TeamIndex, int32 NewScore, int32 PlayerIndex);
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<class UGoalWidget> GoalWidgetClass;
	
	UPROPERTY()
	class UGoalWidget* GoalWidget;
	
	FTimerHandle AddWidgetTimer;
	FTimerHandle GoalAnimTimer;
	
	UPROPERTY(Replicated)
	int32 ScorerIndex = -1;
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	void AddWidget();
	void RemoveWidget();
};
