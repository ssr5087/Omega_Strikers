// Fill out your copyright notice in the Description page of Project Settings.


#include "SessionItemWidget.h"
#include "OSGameInstance.h"
#include "Components/Button.h"


void USessionItemWidget::Set(const struct FSessionInfo& SessionInfo)
{
	txt_roomName->SetText(FText::FromString(SessionInfo.roomName));
	txt_hostName->SetText(FText::FromString(SessionInfo.hostName));
	txt_playerCount->SetText(FText::FromString(SessionInfo.playerCount));
	txt_pingSpeed->SetText(FText::FromString(FString::Printf(TEXT("%dms"), SessionInfo.pingSpeed)));
	
	
	sessionNumber = SessionInfo.index;
}

void USessionItemWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	btn_join->OnClicked.AddDynamic(this, &USessionItemWidget::JoinSession);
	
}

void USessionItemWidget::JoinSession()
{
	auto gi = Cast<UOSGameInstance>(GetWorld()->GetGameInstance());
	if (gi)
	{
		gi->JoinSelectedSession(sessionNumber);
	}
}
