// Fill out your copyright notice in the Description page of Project Settings.


#include "OSGameInstance.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"

void UOSGameInstance::Init()
{
	Super::Init();

	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		SessionInterface = Subsystem->GetSessionInterface();
	}
}

void UOSGameInstance::CreateMySession()
{
	if (!SessionInterface.IsValid()) return;

	FOnlineSessionSettings Settings;
	// Lan 전용게임 설정
	Settings.bIsLANMatch = true;
	// 최대 인원
	Settings.NumPublicConnections = 6;
	// 세션을 다른 플레이어에게 공개할지 여부, True-> 세션 검색으로 찾을 수 있음
	Settings.bShouldAdvertise = true;
	// Presence 기반 매칭 사용 여부 -> Presence : 지금 유저가 어떤 상태인지
	Settings.bUsesPresence = true;
	
	Settings.Set(
	FName("ROOM_NAME"),
	FString("내 방"),
	EOnlineDataAdvertisementType::ViaOnlineServiceAndPing
);

	SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(
		this, &UOSGameInstance::OnCreateSessionComplete);

	SessionInterface->CreateSession(0, NAME_GameSession, Settings);
}

void UOSGameInstance::OnCreateSessionComplete(FName SessionName, bool bSuccess)
{
	UE_LOG(LogTemp, Warning, TEXT("세션 생성 결과: %d"), bSuccess);
	
	if (bSuccess)
	{
		// 서버 시작 + 맵 이동
		GetWorld()->ServerTravel("/Game/Maps/Lobby?listen");
	}
}

void UOSGameInstance::FindMySession()
{
	if (!SessionInterface.IsValid()) return;

	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	// Lan 방에서만 찾겠다
	SessionSearch->bIsLanQuery = true;

	SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(
		this, &UOSGameInstance::OnFindSessionComplete);

	SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
}

void UOSGameInstance::OnFindSessionComplete(bool bSuccess)
{
    if (!bSuccess || SessionSearch->SearchResults.Num() == 0) return;

    SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(
        this, &UOSGameInstance::OnJoinSessionComplete);

    SessionInterface->JoinSession(
        0,
        NAME_GameSession,
        SessionSearch->SearchResults[0]
    );
	
	CachedResults = SessionSearch->SearchResults;

	// UI에 알려주기 (중요)
	OnSessionListUpdated.Broadcast();
}

void UOSGameInstance::OnJoinSessionComplete(
	FName SessionName,
	EOnJoinSessionCompleteResult::Type Result)
{
	FString Address;

	if (SessionInterface->GetResolvedConnectString(SessionName, Address))
	{
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (PC)
		{
			PC->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
		}
	}
}

void UOSGameInstance::JoinSessionByIndex(int32 Index)
{
	if (!SessionInterface.IsValid()) return;
	if (!CachedResults.IsValidIndex(Index)) return;

	SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(
		this, &UOSGameInstance::OnJoinSessionComplete);

	SessionInterface->JoinSession(
		0,
		NAME_GameSession,
		CachedResults[Index]
	);
}