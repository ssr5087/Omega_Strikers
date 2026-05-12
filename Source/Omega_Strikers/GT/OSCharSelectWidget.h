// Fill out your copyright notice in the Description page of Project Settings.
// 천전천승 — 캐릭터 선택 화면
// CharacterStat DataTable에서 고유 캐릭터 이름을 자동 추출

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Omega_Strikers/SSR/CharacterStat.h"
#include "OSCharSelectWidget.generated.h"

class AOSPlayerState;
struct FCharacterStat;
class UOSCharCardWidget;
class UButton;
class UUniformGridPanel;
class UScrollBox;
class UTextBlock;
class UImage;
class AOSCharSelectGameState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterConfirmed, FName, CharacterID);

UCLASS()
class OMEGA_STRIKERS_API UOSCharSelectWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable)
	FOnCharacterConfirmed OnConfirmed;
	
	UFUNCTION(BlueprintPure)
	FName GetSelectedID() const { return SelectedID; }
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
private:
	// --- Bind Widget ---
	// 확정 버튼
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ConfirmButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ConfirmButtonText;

	// 취소 버튼 (확정 해제)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CancelButton;
	
	UPROPERTY(meta = (BindWidget))
	UImage* PreviewImage;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PreviewName;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* StatHPText;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* StatPowerText;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* StatSpeedText;
	
	UPROPERTY(meta = (BindWidget))
	UScrollBox* GridScrollBox;
	
	UPROPERTY(meta = (BindWidget))
	UUniformGridPanel* CharacterGrid;
	
	UPROPERTY(meta = (BindWidget))
	UButton* BackButton;
	
	// ★ 호스트 전용 — 게임 시작 버튼
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> StartGameButton;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> StartGameButtonText;
	
	// 상태 표시
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> StatusText;
	
	// --- 설정 (WBP Details) ---
	UPROPERTY(EditDefaultsOnly, Category="CharSelect")
	TSubclassOf<UOSCharCardWidget> CardWidgetClass;
	
	// 기존 CharacterStat DataTable 그대로 연결
	UPROPERTY(EditDefaultsOnly, Category="CharSelect")
	UDataTable* CharacterStatTable;
	
	UPROPERTY(EditDefaultsOnly, Category="CharSelect")
	int32 Columns = 3;
	
	// ★ 텍스처 매핑
	// 캐릭터 ID(Aimi 등) → 텍스처 내부명(MagicalPlaymaker 등)
	// Details 패널에서 한 번만 세팅
	UPROPERTY(EditDefaultsOnly, Category = "CharSelect|Texture")
	TMap<FName, FName> CharToTextureName;
 
	// 초상화(카드용) 경로: T_UI_Portrait_CloseUp_{내부명}
	UPROPERTY(EditDefaultsOnly, Category = "CharSelect|Texture")
	FString CloseUpPath = TEXT("/Game/Resource/UI/Art/Characters/CloseUp");
 
	// 전신(프리뷰용) 경로: T_UI_Portrait_Full_{내부명}
	UPROPERTY(EditDefaultsOnly, Category = "CharSelect|Texture")
	FString FullPath = TEXT("/Game/Resource/UI/Art/Characters/Full");
	
	// --- 내부 ---
	FName SelectedID;
	FName CurrentSelection = NAME_None;

	UPROPERTY()
	TArray<TObjectPtr<UOSCharCardWidget>> CardWidgets;

	// 캐릭터 ID -> Lv.1 스탯 캐시
	TMap<FName, FCharacterStat> StatCache;
	
	UPROPERTY()
	TObjectPtr<AOSCharSelectGameState> CachedGameState;
	
	// CharacterStat에서 고유 캐릭터 이름 + Lv.1 스탯 추출
	void ExtractUniqueCharacters(TArray<FName>& OutNames, TMap<FName, FCharacterStat>& OutStats);
	UTexture2D* LoadCharIconTexture(FName CharacterID, const FString& Suffix);
	
	void BuildGrid();
	void UpdatePreview(FName CharacterID);
	void ClearSelections();
	
	// 네트워크 콜백
	UFUNCTION()
	void OnCharSelectListUpdated();

	void RefreshCardStates();

	UFUNCTION()
	void OnMySelectRejected(FName CharacterID, const FString& Reason);
	
	UFUNCTION()
	void OnCardClicked(FName CharacterID);
	
	UFUNCTION()
	void OnBackClicked();
	
	// 확정 버튼 (서버에 락 요청)
	UFUNCTION()
	void OnConfirmClicked();
	
	// 확정 취소 버튼
	UFUNCTION()
	void OnCancelClicked();
	
	// 확정/취소 버튼 상태 갱신
	void UpdateButtonStates();

	// ★ 게임 시작 버튼 (호스트 전용)
	UFUNCTION()
	void OnStartGameClicked();
	void UpdateStartGameButton();
	
	UFUNCTION()
	void OnMyConfirmChanged(AOSPlayerState* Player, bool bConfirmed);

	FTimerHandle BindTimerHandle;

	void TryBindAll();
	
	bool bPlayerStateBound = false;
};
