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
	bIsHostedSessionEntry = false;
	
	if (UOSGameInstance* GI = GetGameInstance<UOSGameInstance>())
	{
		if (!GI->CachedResults.IsValidIndex(SessionIndex)) return;

		if (RoomNameText)
		{
			RoomNameText->SetText(FText::FromString(GI->BuildSessionDisplayText(SessionIndex)));
		}
	}
}

void USessionItemWidget::SetupHostedSession()
{
	SessionIndex = INDEX_NONE;
	bIsHostedSessionEntry = true;

	if (UOSGameInstance* GI = GetGameInstance<UOSGameInstance>())
	{
		if (RoomNameText)
		{
			RoomNameText->SetText(FText::FromString(GI->BuildHostedSessionDisplayText()));
		}
	}

	if (JoinButton)
	{
		JoinButton->SetIsEnabled(false);
	}
}

void USessionItemWidget::OnClicked()
{
	if (bIsHostedSessionEntry)
	{
		return;
	}

	if (UOSGameInstance* GI = GetGameInstance<UOSGameInstance>())
	{
		GI->JoinSessionByIndex(SessionIndex);
	}
}
