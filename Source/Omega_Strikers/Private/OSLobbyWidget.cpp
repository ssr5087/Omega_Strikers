// Fill out your copyright notice in the Description page of Project Settings.


#include "OSLobbyWidget.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "OSGameInstance.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameStateBase.h"
#include "Online/OnlineSessionNames.h"


bool UOSLobbyWidget::Initialize()
{
    const bool bSuccess = Super::Initialize();

    if (HostButton)
    {
        HostButton->OnClicked.AddDynamic(this, &UOSLobbyWidget::OnClickHost);
    }

    if (FindButton)
    {
        FindButton->OnClicked.AddDynamic(this, &UOSLobbyWidget::OnClickFind);
    }

    return bSuccess;
}

void UOSLobbyWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (UOSGameInstance* GI = GetGameInstance<UOSGameInstance>())
    {
        GI->OnSessionListUpdated.AddDynamic(this, &UOSLobbyWidget::RefreshSessionList);

        if (GI->SessionInterface.IsValid() &&
            GI->SessionInterface->GetNamedSession(NAME_GameSession) != nullptr)
        {
            GI->FindMySession();
        }
    }

    RefreshSessionList();
    RefreshCurrentPlayerCount();
}

void UOSLobbyWidget::NativeDestruct()
{
    if (UOSGameInstance* GI = GetGameInstance<UOSGameInstance>())
    {
        GI->OnSessionListUpdated.RemoveDynamic(this, &UOSLobbyWidget::RefreshSessionList);
    }

    Super::NativeDestruct();
}

void UOSLobbyWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    RefreshCurrentPlayerCount();
}

void UOSLobbyWidget::OnClickHost()
{
    UOSGameInstance* GI = GetGameInstance<UOSGameInstance>();

    if (GI)
    {
        GI->CreateMySession();
    }
}

void UOSLobbyWidget::OnClickFind()
{
    if (UOSGameInstance* GI = GetGameInstance<UOSGameInstance>())
    {
        GI->FindMySession();
    }
}

void UOSLobbyWidget::AddSessionItem(int32 Index)
{
    if (!SessionList || !SessionItemClass) return;

    UWorld* World = GetWorld();
    if (!World) return;

    USessionItemWidget* Item = CreateWidget<USessionItemWidget>(
        World,
        SessionItemClass
    );

    if (Item)
    {
        Item->Setup(Index);           // ⭐ 인덱스 전달
        SessionList->AddChild(Item);  // ⭐ ScrollBox에 추가
    }
}

void UOSLobbyWidget::AddHostedSessionItem()
{
    if (!SessionList || !SessionItemClass) return;

    UWorld* World = GetWorld();
    if (!World) return;

    USessionItemWidget* Item = CreateWidget<USessionItemWidget>(World, SessionItemClass);
    if (Item)
    {
        Item->SetupHostedSession();
        SessionList->AddChild(Item);
    }
}

void UOSLobbyWidget::RefreshSessionList()
{
    if (!SessionList) return;

    SessionList->ClearChildren();

    if (UOSGameInstance* GI = GetGameInstance<UOSGameInstance>())
    {
        if (GI->CachedResults.Num() == 0 && GI->HasHostedSession())
        {
            AddHostedSessionItem();
            return;
        }

        for (int32 i = 0; i < GI->CachedResults.Num(); i++)
        {
            AddSessionItem(i);
        }
    }
}

void UOSLobbyWidget::RefreshCurrentPlayerCount()
{
    if (!CurrentPlayerCountText)
    {
        return;
    }

    const AGameStateBase* GameState = GetWorld() ? GetWorld()->GetGameState() : nullptr;
    const int32 CurrentPlayers = GameState ? GameState->PlayerArray.Num() : 0;

    CurrentPlayerCountText->SetText(
        FText::FromString(FString::Printf(TEXT("Current Players: %d"), CurrentPlayers)));
}
