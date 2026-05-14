// Fill out your copyright notice in the Description page of Project Settings.
// 플레이어별 상태 리플리케이션 구현
// =====================================================
// OSPlayerState.cpp — 팀 선택 기능 추가 버전
// =====================================================
// 추가된 함수:
//   Server_RequestTeamChange_Implementation
//   Client_OnTeamChangeRejected_Implementation
// =====================================================

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

// ═══════════════════════════════════════════════════════
// ★ 신규 — 팀 변경 요청 (클라이언트 → 서버)
// ═══════════════════════════════════════════════════════
void AOSPlayerState::Server_RequestTeamChange_Implementation(int32 NewTeamID)
{
	// 유효성 체크 — 0(A) 또는 1(B)만 허용
	if (NewTeamID != 0 && NewTeamID != 1)
	{
		Client_OnTeamChangeRejected(TEXT("잘못된 팀 번호입니다."));
		return;
	}

	// 이미 같은 팀이면 무시
	if (TeamID == NewTeamID)
	{
		return;
	}

	// 캐릭터 확정 상태면 팀 변경 불가
	if (bCharacterConfirmed)
	{
		Client_OnTeamChangeRejected(TEXT("캐릭터 확정 후에는 팀을 변경할 수 없습니다."));
		return;
	}

	// CharSelectGameMode가 있으면 인원 검증
	AOSCharSelectGameMode* gm = GetWorld()->GetAuthGameMode<AOSCharSelectGameMode>();
	if (gm)
	{
		bool bOK = gm->TryChangeTeam(this, NewTeamID);
		if (!bOK)
		{
			Client_OnTeamChangeRejected(TEXT("해당 팀이 이미 가득 찼습니다."));
			return;
		}
	}

	// 검증 통과 → 팀 변경
	SetTeamID(NewTeamID);
	LOG_GT(TEXT("%s → Team %d 변경 완료"), *GetPlayerName(), NewTeamID);
}

// ═══════════════════════════════════════════════════════
// ★ 신규 — 팀 변경 거부 알림 (서버 → 클라이언트)
// ═══════════════════════════════════════════════════════
void AOSPlayerState::Client_OnTeamChangeRejected_Implementation(const FString& Reason)
{
	LOG_GT(TEXT("팀 변경 거부: %s"), *Reason);
	OnTeamChangeRejected.Broadcast(Reason);
}

// ═══════════════════════════════════════════════════════
// 캐릭터 선택 (클라이언트 → 서버 RPC)
// ═══════════════════════════════════════════════════════
void AOSPlayerState::Server_SelectCharacter_Implementation(FName CharacterID)
{
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
	
	bool bOK = gm->TryConfirmCharacter(this);
	LOG_GT(TEXT("▶ TryConfirmCharacter 결과: %s"), bOK ? TEXT("SUCCESS") : TEXT("FAIL"));
	if ( bOK )
	{
		bCharacterConfirmed = true;
	}
	else Client_OnSelectRejected(SelectedCharacter, TEXT("확정 실패 — 캐릭터를 먼저 선택하거나 중복을 확인하세요."));
}

void AOSPlayerState::Server_RequestCancelConfirm_Implementation()
{
	AOSCharSelectGameMode* gm = GetWorld()->GetAuthGameMode<AOSCharSelectGameMode>();
	if ( !gm ) return;

	gm->CancelConfirmCharacter(this);
	bCharacterConfirmed = false;
}

void AOSPlayerState::Client_OnSelectRejected_Implementation(FName CharacterID, const FString& Reason)
{
	LOG_GT_W(TEXT("선택 거부: %s (%s)"), *CharacterID.ToString(), *Reason);
	OnSelectRejected.Broadcast(CharacterID, Reason);
}
