// Fill out your copyright notice in the Description page of Project Settings.
// 매치 상태 리플리케이션 구현

#include "OSGameState.h"

#include "Net/UnrealNetwork.h"
#include "Omega_Strikers/Omega_Strikers.h"

AOSGameState::AOSGameState()
{
}

void AOSGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AOSGameState, MatchPhase);
	DOREPLIFETIME(AOSGameState, TeamARoundScore);
	DOREPLIFETIME(AOSGameState, TeamBRoundScore);
	DOREPLIFETIME(AOSGameState, MatchWinner);
	DOREPLIFETIME(AOSGameState, bGoalSequenceActive);
	DOREPLIFETIME(AOSGameState, LastScoringTeam);
	DOREPLIFETIME(AOSGameState, GoalSequenceEndTime);
}

// ═══════════════════════════════════════════════════════
// 매치 단계
// ═══════════════════════════════════════════════════════
void AOSGameState::SetMatchPhase(EOSMatchPhase NewPhase)
{
	if (!HasAuthority()) return;
	
	MatchPhase = NewPhase;
	
	// 서버에서도 델리게이트 호출
	OnMatchPhaseChanged.Broadcast(NewPhase);
}

void AOSGameState::OnRep_MatchPhase()
{
	// 클라에서 단계 변경 시 UI 갱신
	OnMatchPhaseChanged.Broadcast(MatchPhase);
	
	LOG_GT(TEXT("OS Client: Match phase -> %d"), static_cast<int32>(MatchPhase));
}

// ═══════════════════════════════════════════════════════
// 라운드 점수
// ═══════════════════════════════════════════════════════
int32 AOSGameState::GetTeamRoundScore(int32 TeamID) const
{
	return TeamID == 0 ? TeamARoundScore : TeamBRoundScore;
}

void AOSGameState::AddScore(int32 TeamID)
{
	if (!HasAuthority()) return;
	
	if (TeamID == 0)
	{
		TeamARoundScore++;
		OnScoreChanged.Broadcast(0, TeamARoundScore);
	}
	else
	{
		TeamBRoundScore++;
		OnScoreChanged.Broadcast(1, TeamBRoundScore);
	}
}

void AOSGameState::ResetRoundScores()
{
	if (!HasAuthority()) return;
	
	TeamARoundScore = 0;
	TeamBRoundScore = 0;
	OnScoreChanged.Broadcast(0, TeamARoundScore);
	OnScoreChanged.Broadcast(1, TeamBRoundScore);
}

void AOSGameState::OnRep_Score()
{
	// 클라에서 점수 변경 시 UI 갱신
	OnScoreChanged.Broadcast(0, TeamARoundScore);
	OnScoreChanged.Broadcast(1, TeamBRoundScore);
}

// ═══════════════════════════════════════════════════════
// 매치 승자
// ═══════════════════════════════════════════════════════
void AOSGameState::SetMatchWinner(int32 TeamID)
{
	if (!HasAuthority()) return;
	MatchWinner = TeamID;
	OnMatchWinnerDeclared.Broadcast(TeamID);
}

void AOSGameState::OnRep_MatchWinner()
{
	if (MatchWinner >= 0)
	{
		OnMatchWinnerDeclared.Broadcast(MatchWinner);
	}
}

void AOSGameState::StartGoalSequence(int32 ScoringTeam, float InGoalSequenceEndTime)
{
	if (!HasAuthority()) return;

	bGoalSequenceActive = true;
	LastScoringTeam = ScoringTeam;
	GoalSequenceEndTime = InGoalSequenceEndTime;
	OnGoalSequenceChanged.Broadcast(true, ScoringTeam);
}

void AOSGameState::ClearGoalSequence()
{
	if (!HasAuthority()) return;

	bGoalSequenceActive = false;
	LastScoringTeam = -1;
	GoalSequenceEndTime = 0.f;
	OnGoalSequenceChanged.Broadcast(false, -1);
}

void AOSGameState::OnRep_GoalSequenceState()
{
	OnGoalSequenceChanged.Broadcast(bGoalSequenceActive, LastScoringTeam);
}
