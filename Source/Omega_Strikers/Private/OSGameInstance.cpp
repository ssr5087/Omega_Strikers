// Fill out your copyright notice in the Description page of Project Settings.

#include "OSGameInstance.h"

#include "Omega_Strikers/Omega_Strikers.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "GameFramework/GameModeBase.h"
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
	
	// ★ 맵 로드 완료 콜백 — CharSelect 맵 로드 후 세션 생성용
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UOSGameInstance::OnPostLoadMap);
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
	sessionSettings.bUsesPresence = !sessionSettings.bIsLANMatch;
	sessionSettings.bUseLobbiesIfAvailable = !sessionSettings.bIsLANMatch;
	
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
	// ★ ServerTravel은 HostAndCreateSession에서 이미 완료됨 — 여기서는 하지 않음
}

void UOSGameInstance::HostAndCreateSession(FString roomName, int32 playerCount)
{
	// 방 정보를 저장해두고
	PendingRoomName = roomName;
	PendingPlayerCount = playerCount;
	bPendingSessionCreate = true;

	LOG_GT_W(TEXT("★ HostAndCreateSession: ServerTravel 시작, 세션은 맵 로드 후 생성 예정"));
	
	// ★ 심리스 트래블 강제 비활성화 — ?listen이 동작하려면 Hard Travel 필수
	if (AGameModeBase* GM = GetWorld()->GetAuthGameMode())
	{
		GM->bUseSeamlessTravel = false;
	}
	
	// 먼저 listen 서버로 맵 이동
	GetWorld()->ServerTravel(TEXT("/Game/Maps/CharSelect?listen"), true);
}

void UOSGameInstance::CreatePendingSession()
{
	if (bPendingSessionCreate)
	{
		bPendingSessionCreate = false;
		LOG_GT_W(TEXT("★ CreatePendingSession: 세션 생성 시작 (Room=%s, Count=%d)"), *PendingRoomName, PendingPlayerCount);
		CreateSession(PendingRoomName, PendingPlayerCount);
	}
}

void UOSGameInstance::OnPostLoadMap(UWorld* LoadedWorld)
{
	if (!LoadedWorld || !bPendingSessionCreate) return;

	FString MapName = LoadedWorld->GetMapName();
	MapName.RemoveFromStart(LoadedWorld->StreamingLevelsPrefix);
	
	LOG_GT_W(TEXT("★ OnPostLoadMap: %s, bPendingSessionCreate: %d"), *MapName, bPendingSessionCreate);

	if (MapName.Contains(TEXT("CharSelect")))
	{
		// ★ 월드가 완전히 초기화된 후 세션 생성 (0.5초 지연)
		FTimerHandle TimerHandle;
		LoadedWorld->GetTimerManager().SetTimer(TimerHandle, [this]()
		{
			LOG_GT_W(TEXT("★ Timer fired → CreatePendingSession"));
			// ★ Listen 서버가 이미 올라온 상태 — 포트가 정상 할당됨
			CreatePendingSession();
		}, 0.5f, false);
	}
}

void UOSGameInstance::FindOtherSession()
{
	// 세션 검색 상태 설정
	onSearchState.Broadcast(true);
	
	sessionSearch = MakeShareable(new FOnlineSessionSearch());
	
	// 1. 세션 검색 조건 설정 (LAN이면 SEARCH_LOBBIES 사용하지 않음)
	FName subsysName = IOnlineSubsystem::Get()->GetSubsystemName();
	if (subsysName != "NULL")
	{
		sessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
	}
	
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
	
	// ★ 기존 세션이 남아있으면 먼저 파괴 후 Join
	if (sessionInterface->GetNamedSession(FName(mySessionName)))
	{
		LOG_GT_W(TEXT("기존 세션 '%s' 파괴 후 Join 시도"), *mySessionName);
		sessionInterface->DestroySession(FName(mySessionName), FOnDestroySessionCompleteDelegate::CreateLambda(
			[this, index](FName SessionName, bool bSuccess)
				{
					LOG_GT_W(TEXT("세션 파괴 %s → Join 재시도"), bSuccess ? TEXT("성공") : TEXT("실패"));
					auto sr = sessionSearch->SearchResults;
					sessionInterface->JoinSession(0, FName(mySessionName), sr[index]);
				}));
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
		
		// ★ NULL 서브시스템에서 포트 0 문제 강제 수정
		url.ReplaceInline(TEXT(":0"), TEXT(":7777"));
		
		LOG_GT_W(TEXT("Join URL (fixed): %s"), *url);
		
		if (url.IsEmpty() == false)
		{
			pc->ClientTravel(url, TRAVEL_Absolute);
		}
	}
	else
	{
		LOG_SR_W(TEXT("Join Session Failed : %d"), result);
		// ★ 실패 시 세션 정리 — 다음 시도가 가능하도록
		sessionInterface->DestroySession(SessionName);
	}
}

void UOSGameInstance::LeaveSession()
{
	IOnlineSubsystem* onlineSub = IOnlineSubsystem::Get();
	if ( !onlineSub ) return;

	IOnlineSessionPtr sessions = onlineSub->GetSessionInterface();
	if ( !sessions.IsValid() ) return;

	FNamedOnlineSession* existingSession = sessions->GetNamedSession(FName(mySessionName));
	if (!existingSession)
	{
		// 세션 자체가 없으면 바로 메인메뉴로
		ReturnToMainMenu();
		return;
	}

	// ★ 호스트인지 판별: bHosting 플래그 또는 세션의 bHosting 상태
	if (existingSession->bHosting)
	{
		// 호스트 → 세션 파괴 (연결된 클라이언트도 자동으로 끊김)
		LOG_GT(TEXT("OS: Host leaving → DestroySession '%s'"), *mySessionName);

		sessions->AddOnDestroySessionCompleteDelegate_Handle(
			FOnDestroySessionCompleteDelegate::CreateUObject(
				this, &UOSGameInstance::OnLeaveSessionDestroyComplete));

		sessions->DestroySession(FName(mySessionName));
	}
	else
	{
		// 클라이언트 → 세션에서 나가기
		LOG_GT(TEXT("OS: Client leaving session '%s'"), *mySessionName);

		sessions->DestroySession(FName(mySessionName));

		// 서버와의 연결 끊기
		APlayerController* pc = GetFirstLocalPlayerController();
		if ( pc )
		{
			pc->ClientTravel(TEXT("/Game/Maps/Lobby"), ETravelType::TRAVEL_Absolute);
		}
	}
}

void UOSGameInstance::OnLeaveSessionDestroyComplete(FName SessionName, bool bSuccess)
{
	LOG_GT(TEXT("OS: Session '%s' destroyed (Success=%d) → Lobby"),
		*SessionName.ToString(), bSuccess);

	// 델리게이트 해제
	IOnlineSubsystem* onlineSub = IOnlineSubsystem::Get();
	if (onlineSub)
	{
		onlineSub->GetSessionInterface()->ClearOnDestroySessionCompleteDelegates(this);
	}

	ReturnToMainMenu();
}

void UOSGameInstance::ReturnToMainMenu()
{
	// 메인메뉴 맵으로 이동
	UWorld* world = GetWorld();
	if ( world )
	{
		// 호스트는 ServerTravel이 아닌 클라이언트 트래블로 이동
		APlayerController* pc = GetFirstLocalPlayerController();
		if ( pc )
		{
			pc->ClientTravel(TEXT("/Game/Maps/Lobby"), ETravelType::TRAVEL_Absolute);
		}
	}
}

void UOSGameInstance::GameToStart()
{
	//GetWorld()->ServerTravel(TEXT("/Game/Maps/CharSelect?listen?port=7777"));
	GetWorld()->ServerTravel(TEXT("/Game/Maps/Arena"));
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

void UOSGameInstance::SaveTeamSelection(const FString& PlayerKey, int32 TeamID)
{
	TeamSelections.Add(PlayerKey, TeamID);
	LOG_GT(TEXT("팀 저장 [%s] -> %d"), *PlayerKey, TeamID);
}

int32 UOSGameInstance::GetTeamSelection(const FString& PlayerKey) const
{
	const int32* found = TeamSelections.Find(PlayerKey);
	return found ? *found : -1;
}

void UOSGameInstance::ClearTeamSelections()
{
	TeamSelections.Empty();
	LOG_GT(TEXT("팀 선택 초기화"));
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


