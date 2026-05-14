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

	/**
	 * 잠금 처리 + 양팀 이름 표시
	 * @param bLocked         잠금 여부 (true면 회색 + 클릭 불가)
	 * @param SameTeamName    같은 팀 확정자 이름 (잠금 시 메인 이름에 표시)
	 * @param OtherTeamName   다른 팀 확정자 이름 (있으면 별도 표시)
	 */
	void SetLocked(bool bLocked,
				   const FString& SameTeamName = FString(),
				   const FString& OtherTeamName = FString());
	
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
	
	// 다른 팀 확정자 이름 표시용 (WBP에 추가 필요)
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* OtherTeamNameText;
	
	UPROPERTY(meta = (BindWidget))
	UBorder* SelectBorder;
	
	UFUNCTION()
	void HandleClicked();
	
	FName CharacterID;
	
	// 잠금 상태 — SetSelected가 덮어쓰지 않도록
	bool bIsLocked = false;
};
