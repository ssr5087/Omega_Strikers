// Fill out your copyright notice in the Description page of Project Settings.
// 골대 트리거 영역

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Omega_Strikers/SM/OSType.h"
#include "GoalZone.generated.h"

UCLASS()
class OMEGA_STRIKERS_API AGoalZone : public AActor
{
	GENERATED_BODY()

public:
	AGoalZone();

	// 골대 소유 팀
	// 왼쪽 골대 = Blue, 오른쪽 골대 = Red 로 배치
	UPROPERTY(EditAnywhere, Category="Goal")
	EOSTeam GoalTeam = EOSTeam::Red;

	UFUNCTION(BlueprintCallable, Category="Goal")
	int32 GetScoringTeam() const
	{
		// 골대 소유 팀의 반대 팀이 득점한다.
		return GoalTeam == EOSTeam::Blue ? 1 : 0;
	}

	UFUNCTION(BlueprintCallable, Category="Goal")
	EOSTeam GetGoalTeam() const { return GoalTeam; }
	
protected:
	UPROPERTY(VisibleAnywhere, Category="Goal")
	class UBoxComponent* GoalTrigger;
	
	virtual void BeginPlay() override;

};
