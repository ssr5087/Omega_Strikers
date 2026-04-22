// Fill out your copyright notice in the Description page of Project Settings.


#include "OSTopDownController.h"

#include "Camera/CameraActor.h"
#include "Kismet/GameplayStatics.h"
#include "Omega_Strikers/Omega_Strikers.h"

void AOSTopDownController::BeginPlay()
{
	Super::BeginPlay();
	
	if (IsLocalController())
	{
		BindTopDownCamera();
	}
}

void AOSTopDownController::BindTopDownCamera()
{
	TArray<AActor*> cameras;
	UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), ACameraActor::StaticClass(), FName("TopDown"), cameras);
	
	if (cameras.Num() > 0)
	{
		SetViewTargetWithBlend(cameras[0]);
		LOG_GT(TEXT("TopDown 카메라 바인딩 완료"));
	}
	else
	{
		LOG_GT_W(TEXT(" 'TopDown' 태그 CameraActor가 레벨에 없다! "))
	}
}
