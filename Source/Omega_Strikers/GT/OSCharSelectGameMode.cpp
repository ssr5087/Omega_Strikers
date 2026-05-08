// Fill out your copyright notice in the Description page of Project Settings.


#include "OSCharSelectGameMode.h"

#include "OSCharSelectGameState.h"
#include "OSCharSelectTypes.h"
#include "OSPlayerState.h"
#include "Omega_Strikers/Omega_Strikers.h"

AOSCharSelectGameMode::AOSCharSelectGameMode()
{
	// 이 레벨 전용 GameState 클래스 지정
	GameStateClass = AOSCharSelectGameState::StaticClass();
	
	// PlayerState는 기존 OSPlayerState 재사용
	PlayerStateClass = AOSPlayerState::StaticClass();
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
	if (gs->IsCharacterLocked(CharacterID))
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
	
	// 레이스 컨디션 방어 - 최종 중복 체크
	// 다른 플레이어가 '같은' 캐릭터를 이미 확정했는지 확인
	for (const FOSCharSelectEntry& entry : gs->CharSelectList)
	{
		if (entry.PlayerIndex != myEntry.PlayerIndex &&
			entry.CharacterID == myEntry.CharacterID &&
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
//  PostLogin — 접속 시 선택 목록에 등록
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
	
	LOG_GT(TEXT("%s 접속 (총 %d 명)"), *ps->GetPlayerName(), gs->CharSelectList.Num());
}

// ═══════════════════════════════════════════════════════
//  전원 확정 → 아레나 이동
// ═══════════════════════════════════════════════════════
void AOSCharSelectGameMode::OnAllPlayersConfirmed()
{
	LOG_GT(TEXT("=== 전원 확정! 아레나 이동 ==="));
	
	UWorld* world = GetWorld();
	if ( world )
	{
		const FString url = ArenaMapPath + TEXT("?listen");
		world->ServerTravel(url);
	}
}
