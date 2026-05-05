// Fill out your copyright notice in the Description page of Project Settings.


#include "OSGameInstance.h"
#include "Engine/LocalPlayer.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"

void UOSGameInstance::Init()
{
	Super::Init();

	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		SessionInterface = Subsystem->GetSessionInterface();
		
		if (SessionInterface.IsValid())
		{
			// 시작할 때 기존 세션 제거
			SessionInterface->DestroySession(NAME_GameSession);
		}
	}
}

void UOSGameInstance::CreateMySession(const FString& RoomName)
{
	if (!SessionInterface.IsValid()) return;

	PendingRoomName = RoomName.IsEmpty()
		? FString::Printf(TEXT("%s's Room"), *GetLocalPlayerNickname())
		: RoomName;

	if (SessionInterface->GetNamedSession(NAME_GameSession) != nullptr)
	{
		bPendingCreateSession = true;
		SessionInterface->ClearOnDestroySessionCompleteDelegates(this);
		SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(
			this, &UOSGameInstance::OnDestroySessionComplete);
		SessionInterface->DestroySession(NAME_GameSession);
		return;
	}

	CreateSessionInternal();
}

void UOSGameInstance::CreateSessionInternal()
{
	if (!SessionInterface.IsValid()) return;

	FOnlineSessionSettings Settings;
	Settings.bIsLANMatch = true;
	Settings.NumPublicConnections = 6;
	Settings.bShouldAdvertise = true;
	Settings.bUsesPresence = false;
	Settings.bAllowJoinInProgress = true;
	Settings.bAllowJoinViaPresence = false;
	Settings.bAllowJoinViaPresenceFriendsOnly = false;

	Settings.Set(
		FName("ROOM_NAME"),
		PendingRoomName,
		EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	Settings.Set(
		FName("HOST_NAME"),
		GetLocalPlayerNickname(),
		EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	SessionInterface->ClearOnCreateSessionCompleteDelegates(this);
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

	SessionInterface->ClearOnCreateSessionCompleteDelegates(this);
}

void UOSGameInstance::OnDestroySessionComplete(FName SessionName, bool bSuccess)
{
	SessionInterface->ClearOnDestroySessionCompleteDelegates(this);

	if (bSuccess && bPendingCreateSession)
	{
		bPendingCreateSession = false;
		CreateSessionInternal();
		return;
	}

	bPendingCreateSession = false;
}

void UOSGameInstance::FindMySession()
{
	if (!SessionInterface.IsValid()) return;

	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->bIsLanQuery = true;
	SessionSearch->MaxSearchResults = 50;

	SessionInterface->ClearOnFindSessionsCompleteDelegates(this);
	SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(
		this, &UOSGameInstance::OnFindSessionComplete);

	SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
}

void UOSGameInstance::OnFindSessionComplete(bool bSuccess)
{
	CachedResults.Reset();

	if (bSuccess && SessionSearch.IsValid())
	{
		CachedResults = SessionSearch->SearchResults;
	}

	OnSessionListUpdated.Broadcast();
}

void UOSGameInstance::OnJoinSessionComplete(
	FName SessionName,
	EOnJoinSessionCompleteResult::Type Result)
{
	SessionInterface->ClearOnJoinSessionCompleteDelegates(this);

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

	SessionInterface->ClearOnJoinSessionCompleteDelegates(this);
	SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(
		this, &UOSGameInstance::OnJoinSessionComplete);

	SessionInterface->JoinSession(
		0,
		NAME_GameSession,
		CachedResults[Index]
	);
}

FString UOSGameInstance::BuildSessionDisplayText(int32 Index) const
{
	if (!CachedResults.IsValidIndex(Index))
	{
		return TEXT("Invalid Session");
	}

	const FOnlineSessionSearchResult& Result = CachedResults[Index];

	FString RoomName = TEXT("Room");
	Result.Session.SessionSettings.Get(FName("ROOM_NAME"), RoomName);

	FString HostName = TEXT("Host");
	Result.Session.SessionSettings.Get(FName("HOST_NAME"), HostName);

	const int32 MaxPlayers = Result.Session.SessionSettings.NumPublicConnections;
	const int32 CurrentPlayers = MaxPlayers - Result.Session.NumOpenPublicConnections;

	return FString::Printf(TEXT("%s [%d/%d] - %s"), *HostName, CurrentPlayers, MaxPlayers, *RoomName);
}

FString UOSGameInstance::BuildHostedSessionDisplayText() const
{
	if (!SessionInterface.IsValid())
	{
		return TEXT("Hosted Session");
	}

	const FNamedOnlineSession* NamedSession = SessionInterface->GetNamedSession(NAME_GameSession);
	if (NamedSession == nullptr)
	{
		return TEXT("Hosted Session");
	}

	FString RoomName = TEXT("Room");
	NamedSession->SessionSettings.Get(FName("ROOM_NAME"), RoomName);

	FString HostName = TEXT("Host");
	NamedSession->SessionSettings.Get(FName("HOST_NAME"), HostName);

	const int32 MaxPlayers = NamedSession->SessionSettings.NumPublicConnections;
	const int32 CurrentPlayers = MaxPlayers - NamedSession->NumOpenPublicConnections;

	return FString::Printf(TEXT("%s [%d/%d] - %s"), *HostName, CurrentPlayers, MaxPlayers, *RoomName);
}

bool UOSGameInstance::HasHostedSession() const
{
	return SessionInterface.IsValid() &&
		SessionInterface->GetNamedSession(NAME_GameSession) != nullptr;
}

FString UOSGameInstance::GetLocalPlayerNickname() const
{
	if (const ULocalPlayer* LocalPlayer = GetFirstGamePlayer())
	{
		const FString Nickname = LocalPlayer->GetNickname();
		if (!Nickname.IsEmpty())
		{
			return Nickname;
		}
	}

	return TEXT("Host");
}
