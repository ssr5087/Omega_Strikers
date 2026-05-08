// Fill out your copyright notice in the Description page of Project Settings.
// 천전천승 — 캐릭터 카드 위젯 (선택 그리드 개별 항목)

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OSCharCardWidget.generated.h"

class UBorder;
class UTextBlock;
class UImage;
class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCardClicked, FName, CharacterID);

UCLASS()
class OMEGA_STRIKERS_API UOSCharCardWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 카드 세팅 (캐릭터 이름만 있으면 동작)
	UFUNCTION(BlueprintCallable)
	void Setup(FName InCharacterID, UTexture2D* InPortrait);
	
	UFUNCTION(BlueprintCallable)
	void SetSelected(bool bSelected);
	
	UFUNCTION(BlueprintPure)
	FName GetCharacterID() const { return CharacterID; }

	void SetLocked(bool bLocked, const FString& Name = FString(""));
	
	UPROPERTY(BlueprintAssignable)
	FOnCardClicked OnClicked;
	
protected:
	virtual void NativeConstruct() override;
	
private:
	UPROPERTY(meta = (BindWidget))
	UButton* CardButton;
	
	UPROPERTY(meta = (BindWidget))
	UImage* PortraitImage;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* NameText;
	
	UPROPERTY(meta = (BindWidget))
	UBorder* SelectBorder;
	
	UFUNCTION()
	void HandleClicked();
	
	FName CharacterID;
	
	// 잠금 상태 — SetSelected가 덮어쓰지 않도록
	bool bIsLocked = false;
};
