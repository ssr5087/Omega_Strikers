// Fill out your copyright notice in the Description page of Project Settings.


#include "OSCharSelectPlayerController.h"

#include "OSCharSelectWidget.h"
#include "Omega_Strikers/Omega_Strikers.h"

AOSCharSelectPlayerController::AOSCharSelectPlayerController()
{
	bShowMouseCursor = true;
}

void AOSCharSelectPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if (!IsLocalController()) return;
	
	CreateCharSelectWidget();
	
	// UI 전용 입력 모드 - 마우스는 위젯에서만 동작
	FInputModeUIOnly inputMode;
	if (WidgetInstance) inputMode.SetWidgetToFocus(WidgetInstance->TakeWidget());
	inputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(inputMode);
}

void AOSCharSelectPlayerController::CreateCharSelectWidget()
{
	if (!CharSelectWidgetClass) return;
	
	WidgetInstance = CreateWidget<UOSCharSelectWidget>(this, CharSelectWidgetClass);
	if (WidgetInstance)
	{
		WidgetInstance->AddToViewport(0);
		LOG_GT(TEXT("위젯 생성 완료"));
	}
}
