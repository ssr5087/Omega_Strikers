// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LunaSkillCool.generated.h"

/**
 * 
 */
UCLASS()
class OMEGA_STRIKERS_API ULunaSkillCool : public UUserWidget
{
	GENERATED_BODY()
	
public:	
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	class UWidgetAnimation* CoreCool;
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	class UWidgetAnimation* PrimCool;
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	class UWidgetAnimation* SecoCool;
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	class UWidgetAnimation* SpecCool;
	
	void LoadCore();
	void LoadPrim();
	void LoadSeco();
	void LoadSpec();
};
