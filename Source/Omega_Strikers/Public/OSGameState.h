// Fill out your copyright notice in the Description page of Project Settings.
// 매치 상태 ( 모든 클라이언트에 리플리케이트 )
// 점수, 라운드, 매치 단계 등 게임 전체 상태 관리

#pragma once

#include "CoreMinimal.h"
#include "OSGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "OSGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchPhaseChanged, EOSMatchPhase, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnScoreChanged, int32, TeamIndex, int32, NewScore, int32, ScorerID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchWinner, int32, WinningTeam);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGoalSequenceChanged, bool, bIsActive, int32, ScoringTeam);

UCLASS()
class OMEGA_STRIKERS_API AOSGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	AOSGameState();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	// ═══════════════════════════════════════════
	// 매치 단계
	// ═══════════════════════════════════════════
	UFUNCTION(BlueprintCallable, Category="OS|Player")
	EOSMatchPhase GetMatchPhase() const { return MatchPhase; }
	
	// 서버 전용 Setter
	void SetMatchPhase(EOSMatchPhase NewPhase);
	
	// ═══════════════════════════════════════════
	// 라운드 점수 (한 라운드 내 골 수)
	// ═══════════════════════════════════════════
	UFUNCTION(BlueprintCallable, Category="OS|State")
	int32 GetTeamRoundScore(int32 TeamID) const;
	
	UPROPERTY(Replicated)
	int32 ScorerIndex = -1; // GM에서 델리게이트로 전달된 변수 저장
	void AddScore(int32 TeamID, int32 ScorerID);
	void ResetRoundScores();
	
	// ═══════════════════════════════════════════
	// 매치 승자
	// ═══════════════════════════════════════════
	UFUNCTION(BlueprintCallable, Category="OS|State")
	int32 GetMatchWinner() const { return MatchWinner; }
	
	void SetMatchWinner(int32 TeamID);

	UFUNCTION(BlueprintCallable, Category="OS|State")
	bool IsGoalSequenceActive() const { return bGoalSequenceActive; }

	UFUNCTION(BlueprintCallable, Category="OS|State")
	int32 GetLastScoringTeam() const { return LastScoringTeam; }

	UFUNCTION(BlueprintCallable, Category="OS|State")
	float GetGoalSequenceEndTime() const { return GoalSequenceEndTime; }

	void StartGoalSequence(int32 ScoringTeam, float InGoalSequenceEndTime);
	void ClearGoalSequence();
	
	// ═══════════════════════════════════════════
	// 델리게이트 (UI 바인딩용)
	// ═══════════════════════════════════════════
	UPROPERTY(BlueprintAssignable, Category="OS|Events")
	FOnMatchPhaseChanged OnMatchPhaseChanged;
	
	UPROPERTY(BlueprintAssignable, Category="OS|Events")
	FOnScoreChanged OnScoreChanged;
	
	UPROPERTY(BlueprintAssignable, Category="OS|Events")
	FOnMatchWinner OnMatchWinnerDeclared;

	UPROPERTY(BlueprintAssignable, Category="OS|Events")
	FOnGoalSequenceChanged OnGoalSequenceChanged;
	
private:
	// ** Replicated 변수
	UPROPERTY(ReplicatedUsing = OnRep_MatchPhase)
	EOSMatchPhase MatchPhase = EOSMatchPhase::WaitingForPlayers;
	
	UPROPERTY(ReplicatedUsing = OnRep_Score)
	int32 TeamARoundScore = 0;
	
	UPROPERTY(ReplicatedUsing = OnRep_Score)
	int32 TeamBRoundScore = 0;
	
	UPROPERTY(ReplicatedUsing = OnRep_MatchWinner)
	int32 MatchWinner = -1; // -1 : 미정

	UPROPERTY(ReplicatedUsing = OnRep_GoalSequenceState)
	bool bGoalSequenceActive = false;

	UPROPERTY(ReplicatedUsing = OnRep_GoalSequenceState)
	int32 LastScoringTeam = -1;

	UPROPERTY(Replicated)
	float GoalSequenceEndTime = 0.f;
	
	// ** OnRep 콜백
	UFUNCTION()
	void OnRep_MatchPhase();
	
	UFUNCTION()
	void OnRep_Score();
	
	UFUNCTION()
	void OnRep_MatchWinner();

	UFUNCTION()
	void OnRep_GoalSequenceState();
};
