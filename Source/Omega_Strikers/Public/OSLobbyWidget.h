// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OSGameInstance.h"
#include "SessionItemWidget.h"
#include "OSLobbyWidget.generated.h"


UCLASS()
class OMEGA_STRIKERS_API UOSLobbyWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(BindWidget))
	class UButton* btn_createRoom;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(BindWidget))
	class UEditableText* edit_roomName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(BindWidget))
	class USlider* slider_playerCount;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(BindWidget))
	class UTextBlock* txt_playerCount;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(BindWidget))
	class UWidgetSwitcher* WidgetSwitcher;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(BindWidget))
	class UButton* btn_createSession;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(BindWidget))
	class UButton* btn_findSession;
	
	UFUNCTION()
	void SwitchCreatePanel();
	
	UFUNCTION()
	void SwitchFindPanel();
	
	// 메인 화면 돌아가기 버튼
	
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	class UButton* btn_createSessionBack;
	
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	class UButton* btn_findSessionBack;
	
	// 방 검색 버튼
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	class UButton* btn_find;
	
	// 검색 중 메세지
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	class UTextBlock* txt_findingMsg;
	
	// 게임 시작 버튼
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	class UButton* btn_gameToStart;
	
	// 방찾기 버튼 클릭시 호출될 콜백
	UFUNCTION()
	void OnClickedFindSession();
	
	// 게임시작
	UFUNCTION()
	void OnClickedGameToStart();
	
	UFUNCTION()
	void BackToMain();
	
	
public:
	// ------------- 세션 슬롯 ---------------
	// Canvas_FindRoom의 스크롤박스 위젯
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UScrollBox* Scroll_RoomList;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class USessionItemWidget> sessionInfoWidget;
	
	UFUNCTION()
	void AddSlotWidget(const struct FSessionInfo& SessionInfo);
	
	// 방 찾기 상태 이벤트 콜백
	UFUNCTION()
	void OnChangeButtonEnable(bool bIsSearching);
	
	UPROPERTY()
	class UOSGameInstance* gi;
	
	
public:
	virtual void NativeConstruct() override;
	
	UFUNCTION()
	void CreateRoom();
	
	// Slider Callback
	UFUNCTION()
	void OnValueChanged(float Value);
	
};
