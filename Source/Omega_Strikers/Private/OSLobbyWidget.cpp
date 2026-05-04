// Fill out your copyright notice in the Description page of Project Settings.


#include "OSLobbyWidget.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "OSGameInstance.h"
#include "Components/TextBlock.h"


bool UOSLobbyWidget::Initialize()
{
    bool Success = Super::Initialize();

    if (HostButton)
    {
        HostButton->OnClicked.AddDynamic(this, &UOSLobbyWidget::OnClickHost);
        UE_LOG(LogTemp, Warning, TEXT("HostButton 연결됨"));
    }

    if (FindButton)
    {
        FindButton->OnClicked.AddDynamic(this, &UOSLobbyWidget::OnClickFind);
    }

    return Success;
}

void UOSLobbyWidget::OnClickHost()
{
    UE_LOG(LogTemp, Warning, TEXT("Host 클릭됨"));
    UOSGameInstance* GI = GetGameInstance<UOSGameInstance>();

    if (GI)
    {
        UE_LOG(LogTemp, Warning, TEXT("GameInstance 있음"));
        GI->CreateMySession();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("GameInstance 없음"));
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

void UOSLobbyWidget::RefreshSessionList()
{
    if (!SessionList) return;

    SessionList->ClearChildren();

    if (UOSGameInstance* GI = GetGameInstance<UOSGameInstance>())
    {
        for (int32 i = 0; i < GI->CachedResults.Num(); i++)
        {
            AddSessionItem(i);
        }
    }
}
