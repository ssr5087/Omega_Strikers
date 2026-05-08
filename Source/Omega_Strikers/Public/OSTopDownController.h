// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Omega_Strikers/SM/OSPlayerController.h"
#include "Omega_Strikers/SSR/EXPOrb.h"
#include "OSTopDownController.generated.h"

/**
 * OSPlayerController를 상속받아 탑뷰 고정 카메라를 바인딩하는 컨트롤러
 * 레벨에 배치된 "TopDown" 태그 CameraActor를 찾아 뷰 타겟으로 세팅
 */
UCLASS()
class OMEGA_STRIKERS_API AOSTopDownController : public AOSPlayerController
{
	GENERATED_BODY()

public:
	AOSTopDownController();
	
protected:
	virtual void BeginPlay() override;
	
private:
	void BindTopDownCamera();

	UPROPERTY()
	int32 CameraBindRetryCount = 0; // 재시도 횟수 제한용
	static constexpr int32 MaxRetryCount = 10;
	
	

};
