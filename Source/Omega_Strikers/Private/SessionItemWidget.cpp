// Fill out your copyright notice in the Description page of Project Settings.


#include "SessionItemWidget.h"
#include "Components/Button.h"
#include "OSGameInstance.h"

bool USessionItemWidget::Initialize()
{
	Super::Initialize();

	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &USessionItemWidget::OnClicked);
	}

	return true;
}

void USessionItemWidget::Setup(int32 InIndex)
{
	SessionIndex = InIndex;
	
	if (UOSGameInstance* GI = GetGameInstance<UOSGameInstance>())
	{
		if (!GI->CachedResults.IsValidIndex(SessionIndex)) return;

		const FOnlineSessionSearchResult& Result = GI->CachedResults[SessionIndex];

		// 🔥 방 이름 가져오기
		FString RoomName;
		Result.Session.SessionSettings.Get(FName("ROOM_NAME"), RoomName);

		// 🔥 인원수
		int32 CurrentPlayers = Result.Session.SessionSettings.NumPublicConnections
			- Result.Session.NumOpenPublicConnections;

		int32 MaxPlayers = Result.Session.SessionSettings.NumPublicConnections;

		FString DisplayText = FString::Printf(
			TEXT("%s (%d / %d)"),
			*RoomName,
			CurrentPlayers,
			MaxPlayers
		);

		if (RoomNameText)
		{
			RoomNameText->SetText(FText::FromString(DisplayText));
		}
	}
}

void USessionItemWidget::OnClicked()
{
	if (UOSGameInstance* GI = GetGameInstance<UOSGameInstance>())
	{
		GI->JoinSessionByIndex(SessionIndex);
	}
}