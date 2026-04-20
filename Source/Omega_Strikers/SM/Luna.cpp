// Fill out your copyright notice in the Description page of Project Settings.


#include "Luna.h"


// Sets default values
ALuna::ALuna()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ALuna::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ALuna::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ALuna::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ALuna::Ready_CoreHit()
{
	
}

void ALuna::Use_CoreHit()
{
	FVector SelfLoc = GetActorLocation();
	
}

void ALuna::Ready_PrimarySkill()
{
	
}

void ALuna::Use_PrimarySkill()
{
	
}

void ALuna::Ready_SecondarySkill()
{
	
}

void ALuna::Use_SecondarySkill()
{
	
}

void ALuna::Ready_SpecialSkill()
{
	
}

void ALuna::Use_SpecialSkill()
{
	
}

void ALuna::Ready_Flip()
{
	
}

void ALuna::Use_Flip()
{
	
}
