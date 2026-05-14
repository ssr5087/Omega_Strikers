// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "Omega_Strikers/SM/OSType.h"
#include "TeamPlayerStart.generated.h"

UCLASS()
class OMEGA_STRIKERS_API ATeamPlayerStart : public APlayerStart
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TeamID = 0;

	UFUNCTION(BlueprintPure, Category="Team")
	EOSTeam GetTeamSide() const
	{
		return TeamID == 0 ? EOSTeam::Blue : EOSTeam::Red;
	}
};
