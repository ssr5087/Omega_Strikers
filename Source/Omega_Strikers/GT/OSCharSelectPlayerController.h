// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "OSCharSelectPlayerController.generated.h"

class UOSCharSelectWidget;

UCLASS()
class OMEGA_STRIKERS_API AOSCharSelectPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AOSCharSelectPlayerController();

protected:
	virtual void BeginPlay() override;

	// WBP_CharSelect 블루프린트 클래스
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UOSCharSelectWidget> CharSelectWidgetClass;

private:
	UPROPERTY()
	TObjectPtr<UOSCharSelectWidget> WidgetInstance;

	void CreateCharSelectWidget();
};
