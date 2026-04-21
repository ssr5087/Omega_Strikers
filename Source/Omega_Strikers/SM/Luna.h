// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerBase.h"
#include "Luna.generated.h"

UCLASS()
class OMEGA_STRIKERS_API ALuna : public APlayerBase
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ALuna();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stat|SkillRange")
	float PrimaryRange = 5000.f;
	
	
	
	
	bool bPrimarySkillCoolDown = false;
	bool bSecondarySkillCoolDown = false;
	bool bSpecialCoolDown = false;
	
	float PrimarySkillCool = 6.5f;
	float SecondarySkillCool = 14.f;
	float SpecialCool = 35.f;
	
	virtual void Ready_CoreHit() override;
	virtual void Ready_PrimarySkill() override;
	virtual void Ready_SecondarySkill() override;
	virtual void Ready_SpecialSkill() override;
	virtual void Ready_Flip() override;
	
	virtual void Use_CoreHit() override;
	virtual void Use_PrimarySkill() override;
	virtual void Use_SecondarySkill() override;
	virtual void Use_SpecialSkill() override;
	virtual void Use_Flip() override;
};
