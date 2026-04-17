// Fill out your copyright notice in the Description page of Project Settings.
// 골대 트리거 영역

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GoalZone.generated.h"

UCLASS()
class OMEGA_STRIKERS_API AGoalZone : public AActor
{
	GENERATED_BODY()

public:
	AGoalZone();

	// 0 = Red 팀 골대 (Blue팀이 여기에 넣으면 Blue팀 득점)
	// 1 = Blue 팀 골대 (Red팀이 여기에 넣으면 Red팀 득점)
	UPROPERTY(EditAnywhere, Category="Goal")
	int32 ScoringTeam = 0;

	UFUNCTION(BlueprintCallable, Category="Goal")
	int32 GetScoringTeam() const { return ScoringTeam; }
	
protected:
	UPROPERTY(VisibleAnywhere, Category="Goal")
	class UBoxComponent* GoalTrigger;
	
	virtual void BeginPlay() override;

};
