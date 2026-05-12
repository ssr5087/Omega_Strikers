// Fill out your copyright notice in the Description page of Project Settings.


#include "OSTopDownController.h"

#include "Camera/CameraActor.h"
#include "Kismet/GameplayStatics.h"
#include "Omega_Strikers/Omega_Strikers.h"

AOSTopDownController::AOSTopDownController()
{
	bAutoManageActiveCameraTarget = false; // Possess해도 뷰타겟 자동변경 하지 않도록 설정
}

void AOSTopDownController::BeginPlay()
{
	Super::BeginPlay();
	
	if (IsLocalController())
	{
		// ★ 반드시 GameAndUI로 설정 — 마우스 커서 + 키보드 입력 둘 다 필요
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetHideCursorDuringCapture(false);
		SetInputMode(InputMode);
		bShowMouseCursor = true;
		
		GetWorldTimerManager().SetTimerForNextTick(this, &AOSTopDownController::BindTopDownCamera);
	}
}

void AOSTopDownController::BindTopDownCamera()
{
	TArray<AActor*> cameras;
	UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), ACameraActor::StaticClass(), FName("TopDown"), cameras);
	
	if (cameras.Num() > 0)
	{
		SetViewTargetWithBlend(cameras[0]);
		CameraBindRetryCount = 0;
		LOG_GT(TEXT("TopDown 카메라 바인딩 완료"));
	}
	else if (CameraBindRetryCount < MaxRetryCount)
	{
		CameraBindRetryCount++;
		GetWorldTimerManager().SetTimerForNextTick(this, &AOSTopDownController::BindTopDownCamera);
	}
	else
	{
		LOG_GT_W(TEXT(" 'TopDown' 태그 CameraActor가 레벨에 없다! "))
	}
}
