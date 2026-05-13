// Fill out your copyright notice in the Description page of Project Settings.

#include "OSGameInstance.h"

#include "Omega_Strikers/Omega_Strikers.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Online/OnlineSessionNames.h"

void UOSGameInstance::Init()
{
	Super::Init();
	
	if (auto subsys = IOnlineSubsystem::Get())
	{
		// 서브시스템으로부터 세션 인터페이스를 가져온다.
		sessionInterface = subsys->GetSessionInterface();
		
		// ★ 기존 바인딩 제거 후 등록 — 중복 방지
		sessionInterface->OnCreateSessionCompleteDelegates.RemoveAll(this);
		sessionInterface->OnFindSessionsCompleteDelegates.RemoveAll(this);
		sessionInterface->OnJoinSessionCompleteDelegates.RemoveAll(this);
		
		sessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UOSGameInstance::OnCreateSessionComplete);
		sessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UOSGameInstance::OnFindSesssionsComplete);
		sessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UOSGameInstance::OnJoinSessionComplete);
	}
}

void UOSGameInstance::CreateSession(FString roomName, int32 playerCount)
{
	// ★ 기존 세션이 있으면 먼저 파괴
	if (sessionInterface->GetNamedSession(FName(mySessionName)))
	{
		LOG_GT_W(TEXT("기존 세션 '%s' 파괴 후 재생성"), *mySessionName);
		sessionInterface->DestroySession(FName(mySessionName),
			FOnDestroySessionCompleteDelegate::CreateLambda(
				[this, roomName, playerCount](FName SessionName, bool bSuccess)
				{
					LOG_GT_W(TEXT("세션 파괴 %s → 재생성"), bSuccess ? TEXT("성공") : TEXT("실패"));
					CreateSession(roomName, playerCount);  // 재귀 호출로 다시 생성
				}));
		return;
	}
	
	//세션설정변수
	FOnlineSessionSettings sessionSettings;
	
	// 1. Dedicated Server 접속여부
	sessionSettings.bIsDedicated = false;
	
	// 2. 랜선(로컬) 매칭을 할지 Steam 매칭을 사용할지 여부
	// DefaultEngine.ini에서의 [OnlineSubsystem]를 불러온다
	FName subsysName = IOnlineSubsystem::Get()->GetSubsystemName();
	sessionSettings.bIsLANMatch = subsysName == "NULL";
	
	// 3. 매칭이 온라인을 통해 노출될지 여부
	// false 이면 초대를 통해서만 입장이 가능하다.
	sessionSettings.bShouldAdvertise = true;
	
	// 4. 온라인 상태(Presence) 정보를 활용할지 여부
	sessionSettings.bUsesPresence = true;
	sessionSettings.bUseLobbiesIfAvailable = true;
	
	// 5. 게임진행중에 참여를 허가할지 여부
	sessionSettings.bAllowJoinViaPresence = true;
	sessionSettings.bAllowJoinInProgress = true;
	
	// 6. 세션에 참여할 수 있는 공개(public) 연결의 최대 허용 수
	LOG_SR_W(TEXT("CreateSession playerCount : %d"), playerCount);
	sessionSettings.NumPublicConnections = playerCount;
	
	// 7. 커스텀 룸네임 설정
	sessionSettings.Set(FName("ROOM_NAME"), roomName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	
	// 8. 호스트네임 설정
	sessionSettings.Set(FName("HOST_NAME"), mySessionName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	
	// netID
	FUniqueNetIdPtr netID =	GetWorld()->GetFirstLocalPlayerFromController()->GetUniqueNetIdForPlatformUser().GetUniqueNetId();
	
	LOG_SR_W(TEXT("Create Session Start : %s"), *mySessionName);
	sessionInterface->CreateSession(*netID, FName(mySessionName), sessionSettings);
}

void UOSGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	LOG_GT_W(TEXT("Session Name : %s, bWasSuccessful : %d"), *mySessionName, bWasSuccessful);
	if (bWasSuccessful)
	{
		UWorld* World = GetWorld();
		if (!World) return;
        
		// ★ 서버(호스트)에서만 트래블 — 클라이언트는 자동으로 따라감
		if (World->GetNetMode() == NM_Client)
		{
			LOG_GT_W(TEXT("클라이언트이므로 ServerTravel 스킵"));
			return;
		}
        
		World->ServerTravel(TEXT("/Game/Maps/CharSelect?listen"));
	}
}

void UOSGameInstance::FindOtherSession()
{
	// 세션 검색 상태 설정
	onSearchState.Broadcast(true);
	
	sessionSearch = MakeShareable(new FOnlineSessionSearch());
	
	// 1. 세션 검색 조건 설정
	sessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
	
	// 2. Lan 여부
	sessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == FName("NULL");
	
	// 3. 최대 검색 세션 수
	sessionSearch->MaxSearchResults = 100;
	
	// 4. 세션 검색
	sessionInterface->FindSessions(0, sessionSearch.ToSharedRef());
}

void UOSGameInstance::OnFindSesssionsComplete(bool bWasSuccessful)
{
	// 찾기 실패시
	if (bWasSuccessful == false || !sessionSearch.IsValid())
	{
		LOG_SR_W(TEXT("Session Search Failed or sessionSearch invalid"));
		onSearchState.Broadcast(false);
		return;
	}
	
	// 세션 검색 결과 배열
	auto results = sessionSearch->SearchResults;
	LOG_SR_W(TEXT("Search Result count : %d"), results.Num());
	
	// 유효성 체크
	for (int i=0; i<results.Num(); i++)
	
	
		// for (auto sr: results)
	{
		auto sr = results[i];
		if (sr.IsValid() == false)
		{
			continue;
		}
		
		// 세션정보 구조체선언
		FSessionInfo sessionInfo;
		sessionInfo.index = i;
		
		// FString roomName;
		sr.Session.SessionSettings.Get(FName("ROOM_NAME"), sessionInfo.roomName);
		// FString hostName;
		sr.Session.SessionSettings.Get(FName("HOST_NAME"), sessionInfo.hostName);
		
		// 세션주인(방장) 이름 -> pc 이름 가져온다
		// FString userName = sr.Session.OwningUserName;
		
		// 입장가능한 플레이어 수
		int32 maxPlayerCount = sr.Session.SessionSettings.NumPublicConnections;
		// 현재 입장한 플레이어 수 (최대 입장수 - 
		int32 currentPlayerCount = maxPlayerCount - sr.Session.NumOpenPublicConnections;
		
		sessionInfo.playerCount = FString::Printf(TEXT("(%d / %d)"), currentPlayerCount, maxPlayerCount);
		// 핑 정보
		//int32 pingSpeed = sr.PingInMs;
		sessionInfo.pingSpeed = sr.PingInMs;
		
		LOG_SR_W(TEXT("%s"), *sessionInfo.ToString());
		
		// PRINTLOG(TEXT("%s : %s(%s) - (%d/%d), %dms"), *roomName, *hostName, *userName, currentPlayerCount, maxPlayerCount, pingSpeed);
		
		
		// 델리게이트로 위젯에 알려주기
		onSearchCompleted.Broadcast(sessionInfo);
		
	}
		onSearchState.Broadcast(false);
}

void UOSGameInstance::JoinSelectedSession(int32 index)
{
	if (!sessionSearch.IsValid() || !sessionSearch->SearchResults.IsValidIndex(index))
	{
		LOG_GT_W(TEXT("JoinSelectedSession: invalid sessionSearch or index %d"), index);
		return;
	}
	
	auto sr = sessionSearch->SearchResults;
	sessionInterface->JoinSession(0, FName(mySessionName), sr[index]);
}

void UOSGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type result)
{
	if (result == EOnJoinSessionCompleteResult::Success)
	{
		auto pc = GetWorld()->GetFirstPlayerController();
		FString url;
		sessionInterface->GetResolvedConnectString(SessionName, url);
		
		LOG_SR_W(TEXT("Join URL : %s"), *url);
		
		if (url.IsEmpty() == false)
		{
			pc->ClientTravel(url, TRAVEL_Absolute);
		}
	}
	else
	{
		LOG_SR_W(TEXT("Join Session Failed : %d"), result);
	}
}

void UOSGameInstance::GameToStart()
{
	GetWorld()->ServerTravel(TEXT("/Game/Maps/CharSelect?listen?port=7777"));
}

// ═══════════════════════════════════════════════════════
//  ★ 캐릭터 선택 저장/조회
// ═══════════════════════════════════════════════════════
FString UOSGameInstance::GetPlayerKey(const APlayerState* PS)
{
	if ( !PS ) return FString();
	
	FUniqueNetIdRepl netId = PS->GetUniqueId();
	if (netId.IsValid()) return netId->ToString();
	
	// LAN 테스트 시 fallback: PlayerId
	return FString::Printf(TEXT("PID_%d"), PS->GetPlayerId());

}

void UOSGameInstance::ClearCharacterSelections()
{
	CharacterSelections.Empty();
	LOG_GT(TEXT("캐릭터 선택 초기화"));
}

void UOSGameInstance::SaveCharacterSelection(const FString& PlayerKey, FName CharacterID)
{
	CharacterSelections.Add(PlayerKey, CharacterID);
	LOG_GT(TEXT("캐릭터 저장 [%s] -> %s"), *PlayerKey, *CharacterID.ToString());
}

FName UOSGameInstance::GetCharacterSelection(const FString& PlayerKey) const
{
	const FName* found = CharacterSelections.Find(PlayerKey);
	return found ? *found : NAME_None;
}


