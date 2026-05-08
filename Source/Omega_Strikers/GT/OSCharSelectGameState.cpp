// Fill out your copyright notice in the Description page of Project Settings.


#include "OSCharSelectGameState.h"

#include "OSPlayerState.h"
#include "Net/UnrealNetwork.h"

AOSCharSelectGameState::AOSCharSelectGameState()
{
}

void AOSCharSelectGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AOSCharSelectGameState, CharSelectList);
}

// ═══════════════════════════════════════════════════════
//  엔트리 검색/추가
// ═══════════════════════════════════════════════════════
FOSCharSelectEntry& AOSCharSelectGameState::FindOrAddEntry(AOSPlayerState* PS)
{
	const int32 pid = PS->GetPlayerId();
	
	for (FOSCharSelectEntry& entry : CharSelectList)
	{
		if (entry.PlayerIndex == pid) return entry;
	}
	
	FOSCharSelectEntry newEntry;
	newEntry.PlayerIndex = pid;
	newEntry.PlayerName = PS->GetPlayerName();
	newEntry.TeamID = PS->GetTeamID();
	
	CharSelectList.Add(newEntry);
	return CharSelectList.Last();
}

// ═══════════════════════════════════════════════════════
//  잠금 체크
// ═══════════════════════════════════════════════════════
bool AOSCharSelectGameState::IsCharacterLocked(FName CharacterID) const
{
	if (CharacterID == NAME_None) return false;
	
	for (const FOSCharSelectEntry& entry : CharSelectList)
	{
		if (entry.CharacterID == CharacterID && entry.bConfirmed) return true;
	}
	return false;
}

FString AOSCharSelectGameState::GetLockerName(FName CharacterID) const
{
	for (const FOSCharSelectEntry& entry : CharSelectList)
	{
		if (entry.CharacterID == CharacterID && entry.bConfirmed) return entry.PlayerName;
	}
	return FString();
}

bool AOSCharSelectGameState::AreAllPlayersConfirmed() const
{
	if (CharSelectList.Num() == 0) return false;
	
	for (const FOSCharSelectEntry& entry : CharSelectList)
	{
		if (!entry.bConfirmed) return false;
	}
	return true;
}

void AOSCharSelectGameState::BroadcastCharSelectUpdate()
{
	// 서버에서도 즉시 UI 갱신 (OnRep은 클라이언트만)
	OnCharSelectListUpdated.Broadcast();
}

// ═══════════════════════════════════════════════════════
//  OnRep — 클라이언트에서 목록 변경 시 UI 갱신
// ═══════════════════════════════════════════════════════
void AOSCharSelectGameState::OnRep_CharSelectList()
{
	OnCharSelectListUpdated.Broadcast();
}
