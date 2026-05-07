// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "SessionItemWidget.generated.h"


UCLASS()
class OMEGA_STRIKERS_API USessionItemWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	class UTextBlock* txt_roomName;
	
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	class UTextBlock* txt_hostName;
	
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	class UTextBlock* txt_playerCount;
	
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	class UTextBlock* txt_pingSpeed;
	
	
	int32 sessionNumber;
	
	void Set(const struct FSessionInfo& SessionInfo);
	
	// 세션 조인
	
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	class UButton* btn_join;
	
	UFUNCTION()
	void JoinSession();
	
	void NativeConstruct() override;
};
