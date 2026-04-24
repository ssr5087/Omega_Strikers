// Fill out your copyright notice in the Description page of Project Settings.


#include "Asher_Special_Projectile.h"


// Sets default values
AAsher_Special_Projectile::AAsher_Special_Projectile()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AAsher_Special_Projectile::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAsher_Special_Projectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

