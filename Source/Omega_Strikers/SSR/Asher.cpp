// Fill out your copyright notice in the Description page of Project Settings.


#include "Asher.h"


// Sets default values
AAsher::AAsher()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AAsher::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAsher::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AAsher::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AAsher::Ready_CoreHit()
{
	Super::Ready_CoreHit();
}

void AAsher::Ready_PrimarySkill()
{
	Super::Ready_PrimarySkill();
}

void AAsher::Ready_SecondarySkill()
{
	Super::Ready_SecondarySkill();
}

void AAsher::Ready_SpecialSkill()
{
	Super::Ready_SpecialSkill();
}

void AAsher::Ready_Flip()
{
	Super::Ready_Flip();
}

void AAsher::Use_CoreHit()
{
	Super::Use_CoreHit();
}

void AAsher::Use_PrimarySkill()
{
	Super::Use_PrimarySkill();
}

void AAsher::Use_SecondarySkill()
{
	Super::Use_SecondarySkill();
}

void AAsher::Use_SpecialSkill()
{
	Super::Use_SpecialSkill();
}

void AAsher::Use_Flip()
{
	Super::Use_Flip();
}
