// Fill out your copyright notice in the Description page of Project Settings.
// 플레이어별 상태 리플리케이션 구현

#include "OSPlayerState.h"

#include "Net/UnrealNetwork.h"
#include "Omega_Strikers/Omega_Strikers.h"

AOSPlayerState::AOSPlayerState()
{
}

void AOSPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AOSPlayerState, TeamID);
	DOREPLIFETIME(AOSPlayerState, SelectedCharacter);
	DOREPLIFETIME(AOSPlayerState, Goals);
	DOREPLIFETIME(AOSPlayerState, Assists);
	DOREPLIFETIME(AOSPlayerState, Knockouts);
	DOREPLIFETIME(AOSPlayerState, bIsReady);
}

// ═══════════════════════════════════════════════════════
// 팀 배정
// ═══════════════════════════════════════════════════════
void AOSPlayerState::SetTeamID(int32 NewTeam)
{
	if (!HasAuthority()) return;
	TeamID = NewTeam;
	
	// 서버에서도 즉시 알림
	OnTeamAssigned.Broadcast(this, TeamID);
}

void AOSPlayerState::OnRep_TeamID()
{
	// 클라에서 팀 변경 시 UI 갱신
	OnTeamAssigned.Broadcast(this, TeamID);
	
	LOG_GT(TEXT("Player assigned to Team %d"), TeamID);
}

// ═══════════════════════════════════════════════════════
// 캐릭터 선택 (클라이언트 → 서버 RPC)
// ═══════════════════════════════════════════════════════
void AOSPlayerState::Server_SelectCharacter_Implementation(FName CharacterID)
{
	// TODO: 중복 선택 방지 검증
	SelectedCharacter = CharacterID;
	
	LOG_GT(TEXT("Player %s selected Character %s"), *GetPlayerName(), *CharacterID.ToString());
}

// ═══════════════════════════════════════════════════════
// 준비 상태
// ═══════════════════════════════════════════════════════
void AOSPlayerState::Server_SetReady_Implementation(bool bReady)
{
	bIsReady = bReady;
	
	LOG_GT(TEXT("Player %s ready = %d"), *GetPlayerName(), bReady);
}

// ═══════════════════════════════════════════════════════
// 인게임 스탯
// ═══════════════════════════════════════════════════════
void AOSPlayerState::AddGoal()
{
	if (!HasAuthority()) return;
	Goals++;
}

void AOSPlayerState::AddAssist()
{
	if (!HasAuthority()) return;
	Assists++;
}

void AOSPlayerState::AddKnockout()
{
	if (!HasAuthority()) return;
	Knockouts++;
}

void AOSPlayerState::ResetMatchStats()
{
	if (!HasAuthority()) return;
	Goals = 0;
	Assists = 0;
	Knockouts = 0;
}




