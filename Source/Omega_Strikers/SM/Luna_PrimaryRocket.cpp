// Fill out your copyright notice in the Description page of Project Settings.


#include "Luna_PrimaryRocket.h"


// Sets default values
ALuna_PrimaryRocket::ALuna_PrimaryRocket()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ALuna_PrimaryRocket::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ALuna_PrimaryRocket::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

