// Fill out your copyright notice in the Description page of Project Settings.


#include "EXPComponent.h"
#include "Omega_Strikers/Public/PlayerBase.h"

#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
UEXPComponent::UEXPComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicated(true);
	
}

void UEXPComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void UEXPComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UEXPComponent, CurrentEXP);
}

// Called every frame
void UEXPComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


void UEXPComponent::Server_AddEXP_Implementation(int32 Amount)
{
	AddEXP(Amount);
}

void UEXPComponent::AddEXP(int32 Amount)
{
	if (!GetOwner()->HasAuthority())
		return;
	
	CurrentEXP += Amount;
	
	APlayerBase* Player = Cast<APlayerBase>(GetOwner());
	if (!Player)
		return;
	
	if (CurrentEXP >= MaxEXP)
	{
		LevelUP();
	}
}

void UEXPComponent::LevelUP()
{
	if (!GetOwner()->HasAuthority())
		return;
	
	APlayerBase* Player = Cast<APlayerBase>(GetOwner());
	if (!Player)
		return;
	
	CurrentEXP = 0;
	
	Player->Level++;
	
	UE_LOG(LogTemp, Warning, TEXT("Level UP! ->  %d"), Player->Level);
	
	// Delegate (서버용)
	OnLevelUp.Broadcast(Player->Level);
}

// Replication

void UEXPComponent::OnRep_CurrentEXP()
{
	// UI용
}



