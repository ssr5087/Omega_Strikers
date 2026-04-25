// Fill out your copyright notice in the Description page of Project Settings.


#include "Aimi.h"


// Sets default values
AAimi::AAimi()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AAimi::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAimi::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AAimi::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

