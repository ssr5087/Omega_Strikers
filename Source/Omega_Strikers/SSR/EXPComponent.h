// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EXPComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class OMEGA_STRIKERS_API UEXPComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UEXPComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	// EXP Logic
	
	UFUNCTION(server, Reliable)
	void Server_AddEXP(int32 Amount);
	
	void AddEXP(int32 Amount);
	
	void LevelUP();
	
	// Replication
	
	UPROPERTY(ReplicatedUsing = OnRep_CurrentEXP)
	int32 CurrentEXP = 0;
	
	UPROPERTY(EditAnywhere)
	int32 MaxEXP = 100;
	
	UFUNCTION()
	void OnRep_CurrentEXP();
	
	// Delegate
	
	UPROPERTY(BlueprintAssignable)
	FOnLevelUp OnLevelUp;
};
