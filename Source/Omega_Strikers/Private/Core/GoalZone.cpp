// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/GoalZone.h"

#include "Components/BoxComponent.h"

AGoalZone::AGoalZone()
{
	PrimaryActorTick.bCanEverTick = false;

	GoalTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("GoalTrigger"));
	GoalTrigger->SetBoxExtent(FVector(1150.0f, 200.0f, 200.0f));
	GoalTrigger->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	GoalTrigger->SetGenerateOverlapEvents(true);
	SetRootComponent(GoalTrigger);
}

void AGoalZone::BeginPlay()
{
	Super::BeginPlay();
}