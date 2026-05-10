// Fill out your copyright notice in the Description page of Project Settings.
// 플레이어별 상태 리플리케이션 구현

#include "OSPlayerState.h"

#include "Net/UnrealNetwork.h"
#include "Omega_Strikers/Omega_Strikers.h"
#include "Omega_Strikers/GT/OSCharSelectGameMode.h"

AOSPlayerState::AOSPlayerState()
{
}

void AOSPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AOSPlayerState, TeamID);
	DOREPLIFETIME(AOSPlayerState, SelectedCharacter);
	DOREPLIFETIME(AOSPlayerState, bCharacterConfirmed);
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

void AOSPlayerState::OnRep_bCharacterConfirmed()
{
	LOG_GT(TEXT("★ OnRep_bCharacterConfirmed: %s, bConfirmed=%d"), *GetPlayerName(), bCharacterConfirmed);
	OnPlayerConfirmChanged.Broadcast(this, bCharacterConfirmed);
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

// ═══════════════════════════════════════════════════════
// RPC
// ═══════════════════════════════════════════════════════
// ═══════════════════════════════════════════════════════
// 캐릭터 선택 — CharSelect 레벨 RPC (GameMode 검증)
// ═══════════════════════════════════════════════════════
void AOSPlayerState::Server_RequestSelectCharacter_Implementation(FName CharacterID)
{
	AOSCharSelectGameMode* gm = GetWorld()->GetAuthGameMode<AOSCharSelectGameMode>();
	
	if ( gm )
	{
		// CharSelect 레벨 → GameMode가 중복 검증
		bool bOK = gm->TrySelectCharacter(this, CharacterID);
		if (bOK)
		{
			SelectedCharacter = CharacterID;
		}
		else
		{
			Client_OnSelectRejected(CharacterID,
				TEXT("이미 다른 플레이어가 선택한 캐릭터입니다."));
		}
	}
	else
	{
		// 다른 레벨 → 기존 로직
		SelectedCharacter = CharacterID;
	}
}

void AOSPlayerState::Server_RequestConfirmCharacter_Implementation()
{
	AOSCharSelectGameMode* gm = GetWorld()->GetAuthGameMode<AOSCharSelectGameMode>();
	LOG_GT(TEXT("▶ ConfirmCharacter: GM=%s, SelectedCharacter=%s"), 
		gm ? TEXT("OK") : TEXT("NULL"), *SelectedCharacter.ToString());
	if ( !gm ) return;
	
	bool bOK = gm->TrySelectCharacter(this, SelectedCharacter);
	LOG_GT(TEXT("▶ TrySelectCharacter 결과: %s"), bOK ? TEXT("SUCCESS") : TEXT("FAIL"));
	if ( bOK )
	{
		bCharacterConfirmed = true;
		OnRep_bCharacterConfirmed();
		
	}
	else Client_OnSelectRejected(SelectedCharacter, TEXT("확정 실패 — 캐릭터를 먼저 선택하거나 중복을 확인하세요."));
}

void AOSPlayerState::Server_RequestCancelConfirm_Implementation()
{
	AOSCharSelectGameMode* gm = GetWorld()->GetAuthGameMode<AOSCharSelectGameMode>();
	if ( !gm ) return;

	gm->CancelConfirmCharacter(this);
	bCharacterConfirmed = false;
	OnRep_bCharacterConfirmed();
}

void AOSPlayerState::Client_OnSelectRejected_Implementation(FName CharacterID, const FString& Reason)
{
	LOG_GT_W(TEXT("선택 거부: %s (%s)"), *CharacterID.ToString(), *Reason);

	OnSelectRejected.Broadcast(CharacterID, Reason);
}
