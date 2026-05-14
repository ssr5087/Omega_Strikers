// Fill out your copyright notice in the Description page of Project Settings.


#include "OSCharSelectGameMode.h"

#include "OSCharSelectGameState.h"
#include "OSCharSelectTypes.h"
#include "OSGameInstance.h"
#include "OSPlayerState.h"
#include "Omega_Strikers/Omega_Strikers.h"

AOSCharSelectGameMode::AOSCharSelectGameMode()
{
	// 이 레벨 전용 GameState 클래스 지정
	GameStateClass = AOSCharSelectGameState::StaticClass();
	
	// PlayerState는 기존 OSPlayerState 재사용
	PlayerStateClass = AOSPlayerState::StaticClass();
	
	bUseSeamlessTravel = true;
}

void AOSCharSelectGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	LOG_GT(TEXT("★ CharSelectGameMode BeginPlay - NetMode: %d"), 
		(int32)GetWorld()->GetNetMode());
	
	// 호스트(서버)에서만 실행
	if (GetWorld()->GetNetMode() != NM_Client)
	{
		if (auto gi = Cast<UOSGameInstance>(GetGameInstance()))
		{
			LOG_GT(TEXT("★ Calling CreatePendingSession"));
			gi->CreatePendingSession();
		}
	}
	
	// GameInstance에서 세션 인원 수 → GameState에 세팅
	if (UOSGameInstance* gi = Cast<UOSGameInstance>(GetGameInstance()))
	{
		if (gi->PendingPlayerCount > 0)
		{
			if (AOSCharSelectGameState* gs = GetGameState<AOSCharSelectGameState>())
			{
				gs->RequiredPlayerCount = gi->PendingPlayerCount;
			}
		}
	}
}

// ═══════════════════════════════════════════════════════
//  선택 요청
// ═══════════════════════════════════════════════════════
bool AOSCharSelectGameMode::TrySelectCharacter(AOSPlayerState* Player, FName CharacterID)
{
	if ( !Player ) return false;
	
	AOSCharSelectGameState* gs = GetGameState<AOSCharSelectGameState>();
	if ( !gs ) return false;
	
	FOSCharSelectEntry& myEntry = gs->FindOrAddEntry( Player );
	
	// 이미 확정 -> 교체 불가
	if (myEntry.bConfirmed)
	{
		LOG_GT(TEXT("%s 이미 확정됨"), *Player->GetPlayerName());
		return false;
	}
	
	// 선택 해제
	if (CharacterID == NAME_None)
	{
		myEntry.CharacterID = NAME_None;
		gs->BroadcastCharSelectUpdate();
		return true;
	}
	
	// 중복 체크: 다른 플레이어가 확정한 캐릭터
	const int32 myTeam = Player->GetTeamID();
	if (gs->IsCharacterLocked(CharacterID, myTeam))
	{
		LOG_GT_W(TEXT("'%s' 이미 다른 플레이어가 확정"), *CharacterID.ToString());
		return false;
	}
	
	// 저장
	myEntry.CharacterID = CharacterID;
	gs->BroadcastCharSelectUpdate();
	
	LOG_GT(TEXT("%s -> %s 선택"), *Player->GetPlayerName(), *CharacterID.ToString());
	return true;
}

// ═══════════════════════════════════════════════════════
//  확정 요청
// ═══════════════════════════════════════════════════════
bool AOSCharSelectGameMode::TryConfirmCharacter(AOSPlayerState* Player)
{
	if ( !Player ) return false;
	
	AOSCharSelectGameState* gs = GetGameState<AOSCharSelectGameState>();
	if ( !gs ) return false;
	
	FOSCharSelectEntry& myEntry = gs->FindOrAddEntry(Player);
	
	// 캐릭터 미선택
	if (myEntry.CharacterID == NAME_None)
	{
		LOG_GT_W(TEXT("%s 캐릭터 미선택, 확정 불가"), *Player->GetPlayerName());
		return false;
	}
	
	// 팀 미배정이면 확정 불가
	if (Player->GetTeamID() == -1)
	{
		LOG_GT_W(TEXT("%s 팀 미배정, 확정 불가"), *Player->GetPlayerName());
		return false;
	}
	
	// 레이스 컨디션 방어 - 최종 중복 체크
	// 팀내 다른 플레이어가 '같은' 캐릭터를 이미 확정했는지 확인
	const int32 myTeam = Player->GetTeamID();
	for (const FOSCharSelectEntry& entry : gs->CharSelectList)
	{
		if (entry.PlayerIndex != myEntry.PlayerIndex &&
			entry.CharacterID == myEntry.CharacterID &&
			entry.TeamID == myTeam &&
			entry.bConfirmed)
		{
			myEntry.CharacterID = NAME_None;
			gs->BroadcastCharSelectUpdate();
			return false;
		}
	}
	
	// 확정
	myEntry.bConfirmed = true;
	
	// PlayerState에도 반영 (아레나 맵으로 넘어갈 때 사용)
	Player->Server_SelectCharacter(myEntry.CharacterID);
	
	LOG_GT(TEXT("%s -> %s 확정!"), *Player->GetPlayerName(), *myEntry.CharacterID.ToString());
	gs->BroadcastCharSelectUpdate();
	
	// 전원 확정 체크
	if (gs->AreAllPlayersConfirmed()) OnAllPlayersConfirmed();
	return true;
}

// ═══════════════════════════════════════════════════════
//  확정 취소
// ═══════════════════════════════════════════════════════
void AOSCharSelectGameMode::CancelConfirmCharacter(AOSPlayerState* Player)
{
	if ( !Player ) return;
	
	AOSCharSelectGameState* gs = GetGameState<AOSCharSelectGameState>();
	if ( !gs ) return;
	
	FOSCharSelectEntry& myEntry = gs->FindOrAddEntry(Player);
	if (!myEntry.bConfirmed) return;
	
	myEntry.bConfirmed = false;
	
	LOG_GT(TEXT("%s 확정 취소"), *Player->GetPlayerName());
	
	gs->BroadcastCharSelectUpdate();
}
// ═══════════════════════════════════════════════════════
//  ★ 신규 — 팀 변경 검증
// ═══════════════════════════════════════════════════════
bool AOSCharSelectGameMode::TryChangeTeam(AOSPlayerState* Player, int32 NewTeamID)
{
	if ( !Player ) return false;
	
	// 유효한 팀 번호 체크
	if ( NewTeamID != 0 && NewTeamID != 1 ) return false;
	
	// 이미 같은 팀이면 성공 (변경 불필요)
	if ( Player->GetTeamID() == NewTeamID ) return true;
	
	// 이동하려는 팀의 현재 인원 체크
	int32 targetCount = GetTeamCount(NewTeamID);
	if (targetCount >= MaxPlayersPerTeam)
	{
		LOG_GT_W(TEXT("%s 팀 %d 변경 거부 — 인원 초과 (%d/%d)"), *Player->GetPlayerName(), NewTeamID, targetCount, MaxPlayersPerTeam);
		return false;
	}
	
	LOG_GT(TEXT("%s 팀 변경 승인: Team %d → Team %d"),
		*Player->GetPlayerName(), Player->GetTeamID(), NewTeamID);
	return true;
}

// ═══════════════════════════════════════════════════════
//  ★ 신규 — 팀 인원 수
// ═══════════════════════════════════════════════════════
int32 AOSCharSelectGameMode::GetTeamCount(int32 TeamID) const
{
	int32 count = 0;

	AGameStateBase* gs = GetGameState<AGameStateBase>();
	if (!gs) return 0;

	for (APlayerState* basePS : gs->PlayerArray)
	{
		AOSPlayerState* ps = Cast<AOSPlayerState>(basePS);
		if (ps && ps->GetTeamID() == TeamID)
		{
			count++;
		}
	}

	return count;
}

// ═══════════════════════════════════════════════════════
//  게임 시작 (호스트 버튼)
// ═══════════════════════════════════════════════════════
void AOSCharSelectGameMode::StartArenaTravel()
{
	// 전원 확정 검증
	AOSCharSelectGameState* gs = GetGameState<AOSCharSelectGameState>();
	if (!gs || !gs->AreAllPlayersConfirmed())
	{
		LOG_GT_W(TEXT("StartArenaTravel: 아직 전원 확정되지 않음"));
		return;
	}

	LOG_GT(TEXT("=== 호스트 게임 시작! 아레나 이동 ==="));
	
	// GameInstance에 캐릭터 선택 저장
	UOSGameInstance* gi = Cast<UOSGameInstance>(GetGameInstance());
	if ( gi )
	{
		gi->ClearCharacterSelections();
		gi->ClearTeamSelections(); // 팀 선택 초기화
		
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			APlayerController* pc = It->Get();
			if ( !pc ) continue;
            
			AOSPlayerState* ps = Cast<AOSPlayerState>(pc->PlayerState);
			if ( !ps ) continue;
            
			FName charID = ps->GetSelectedCharacter();
			if ( charID.IsNone() ) continue;
			
			FString playerKey = UOSGameInstance::GetPlayerKey(ps);
			gi->SaveCharacterSelection(playerKey, charID);
			gi->SaveTeamSelection(playerKey, ps->GetTeamID()); // 팀 저장
			
			LOG_GT(TEXT("저장 : [%s] → %s"), *playerKey, *charID.ToString());
		}
		
		LOG_GT(TEXT("GameInstance에 %d명 캐릭터 선택 저장 완료"), gi->GetAllSelections().Num());
	}
	else
	{
		LOG_GT_W(TEXT("GameInstance 캐스팅 실패! Project Settings → Game Instance Class 확인"));
	}
	
	// 아레나로 이동
	UWorld* world = GetWorld();
	if ( world )
	{
		FString url = ArenaMapPath + TEXT("?listen");
		
		if ( ArenaGameModeClass )
		{
			const FString gmPath = ArenaGameModeClass->GetPathName();
			url = FString::Printf(TEXT("%s?listen?game=%s"), *ArenaMapPath, *gmPath);
		}
    
		LOG_GT(TEXT("ServerTravel URL: %s"), *url);
		world->ServerTravel(url);
	}
}

// ═══════════════════════════════════════════════════════
//  PostLogin — 접속 시 선택 목록에 등록 + 팀 자동 배정
// ═══════════════════════════════════════════════════════
void AOSCharSelectGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	if ( !NewPlayer ) return;
	
	AOSCharSelectGameState* gs = GetGameState<AOSCharSelectGameState>();
	AOSPlayerState* ps = Cast<AOSPlayerState>(NewPlayer->PlayerState);
	if (!gs || !ps ) return;
	
	// 선택 목록에 빈 엔트리 추가
	gs->FindOrAddEntry(ps);
	gs->BroadcastCharSelectUpdate();
	
	// 세션의 최대 인원수를 RequiredPlayerCount에 세팅
	UOSGameInstance* GI = Cast<UOSGameInstance>(GetGameInstance());
	if (GI && GI->sessionInterface.IsValid())
	{
		FNamedOnlineSession* NamedSession = GI->sessionInterface->GetNamedSession(FName(GI->mySessionName));
		if (NamedSession)
		{
			gs->RequiredPlayerCount = NamedSession->SessionSettings.NumPublicConnections;
		}
	}
	// 접속 시 인원 적은 팀에 자동 배정 (나중에 UI에서 변경 가능)
	AssignDefaultTeam(ps);
	
	LOG_GT(TEXT("%s 접속 (총 %d 명)"), *ps->GetPlayerName(), gs->CharSelectList.Num());
}

// ═══════════════════════════════════════════════════════
//  접속 시 자동 팀 배정
// ═══════════════════════════════════════════════════════
void AOSCharSelectGameMode::AssignDefaultTeam(AOSPlayerState* PS)
{
	if ( !PS ) return;
	
	// 이미 팀이 배정되어 있으면 스킵 (SeamlessTravel 등)
	if ( PS->GetTeamID() != -1 ) return;
	
	int32 teamBlueCount = GetTeamCount(0);
	int32 teamRedCount = GetTeamCount(1);
	
	// 인원 적은 팀에 배정, 같으면 블루팀
	int32 assignedTeam = (teamRedCount < teamBlueCount) ? 1 : 0;
	PS->SetTeamID(assignedTeam);
	
	LOG_GT(TEXT("%s → Team %d 자동 배정 (A:%d, B:%d)"),
		*PS->GetPlayerName(), assignedTeam, teamBlueCount, teamRedCount);
}

// ═══════════════════════════════════════════════════════
//  전원 확정 → 아레나 이동
// ═══════════════════════════════════════════════════════
void AOSCharSelectGameMode::OnAllPlayersConfirmed()
{
	LOG_GT(TEXT("=== 전원 확정! 아레나 이동 ==="));
	// StartArenaTravel()에서 처리하므로 여기에서는 로그만 출력
}