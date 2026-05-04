// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SessionItemWidget.h"
#include "OSLobbyWidget.generated.h"


UCLASS()
class OMEGA_STRIKERS_API UOSLobbyWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual bool Initialize() override;

	// 버튼 바인딩
	UPROPERTY(meta = (BindWidget))
	class UButton* HostButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* FindButton;

	UPROPERTY(meta = (BindWidget))
	class UScrollBox* SessionList;

	// 클릭 함수
	UFUNCTION()
	void OnClickHost();

	UFUNCTION()
	void OnClickFind();
	
	
	void AddSessionItem(int32 Index);
	
	// UI에서 세션 목록 받기
	UFUNCTION(BlueprintCallable)
	void RefreshSessionList();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI")
	TSubclassOf<class USessionItemWidget> SessionItemClass;
};
