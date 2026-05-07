// Fill out your copyright notice in the Description page of Project Settings.


#include "OSLobbyWidget.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "OSGameInstance.h"
#include "Components/EditableText.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"


void UOSLobbyWidget::SwitchCreatePanel()
{
	//Widget 활성화
	WidgetSwitcher->SetActiveWidgetIndex(1);
}

void UOSLobbyWidget::SwitchFindPanel()
{
	//Widget 활성화
	WidgetSwitcher->SetActiveWidgetIndex(2);
	// find 안누르고 처음에 find 찾고 시작한다.
	OnClickedFindSession();
}

void UOSLobbyWidget::OnClickedFindSession()
{
	Scroll_RoomList->ClearChildren();
	
	if (gi != nullptr)
	{
		gi->FindOtherSession();
	}
}

void UOSLobbyWidget::OnClickedGameToStart()
{
	if (gi)
	{
		gi->GameToStart();
	}
}

void UOSLobbyWidget::BackToMain()
{
	//Widget 활성화
	WidgetSwitcher->SetActiveWidgetIndex(0);
}

void UOSLobbyWidget::OnChangeButtonEnable(bool bIsSearching)
{
	// 검색버튼 비활성화
	btn_find->SetIsEnabled(!bIsSearching);

	if (bIsSearching)
	{
		// 검색중 보이도록 처리
		txt_findingMsg->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		// 검색 끝나면 사라지도록 처리
		txt_findingMsg->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UOSLobbyWidget::AddSlotWidget(const struct FSessionInfo& SessionInfo)
{
	auto slot = CreateWidget<USessionItemWidget>(this, sessionInfoWidget);
	slot->Set(SessionInfo);
	
	Scroll_RoomList->AddChild(slot);
}

void UOSLobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	gi = Cast<UOSGameInstance>(GetWorld()->GetGameInstance());
	gi->onSearchCompleted.AddDynamic(this, &UOSLobbyWidget::AddSlotWidget);
	gi->onSearchState.AddDynamic(this, &UOSLobbyWidget::OnChangeButtonEnable);
	
	btn_createRoom->OnClicked.AddDynamic(this, &UOSLobbyWidget::CreateRoom);
	slider_playerCount->OnValueChanged.AddDynamic(this, &UOSLobbyWidget::OnValueChanged);
	
	btn_createSession->OnClicked.AddDynamic(this, &UOSLobbyWidget::SwitchCreatePanel);
	btn_findSession->OnClicked.AddDynamic(this, &UOSLobbyWidget::SwitchFindPanel);
	
	btn_createSessionBack->OnClicked.AddDynamic(this, &UOSLobbyWidget::BackToMain);
	btn_findSessionBack->OnClicked.AddDynamic(this, &UOSLobbyWidget::BackToMain);
	btn_find->OnClicked.AddDynamic(this, &UOSLobbyWidget::OnClickedFindSession);
	
	btn_gameToStart->OnClicked.AddDynamic(this, &UOSLobbyWidget::OnClickedGameToStart);
	
}

void UOSLobbyWidget::CreateRoom()
{
	if (gi && edit_roomName->GetText().IsEmpty() == false)
	{
		FString roomName = edit_roomName->GetText().ToString();
		int32 playerCount = slider_playerCount->GetValue();
		gi->CreateSession(roomName, playerCount);
	}
}

void UOSLobbyWidget::OnValueChanged(float Value)
{
	txt_playerCount->SetText(FText::AsNumber(Value));
}
