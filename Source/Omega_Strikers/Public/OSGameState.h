// Fill out your copyright notice in the Description page of Project Settings.
// 매치 상태 ( 모든 클라이언트에 리플리케이트 )
// 점수, 라운드, 매치 단계 등 게임 전체 상태 관리

#pragma once

#include "CoreMinimal.h"
#include "OSGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "OSGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchPhaseChanged, EOSMatchPhase, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnScoreChanged, int32, NAME_Team, int32, NewScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchWinner, int32, WinningTeam);

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
	
	void AddScore(int32 TeamID);
	void ResetRoundScores();
	
	// ═══════════════════════════════════════════
	// 라운드 승수 (3판 2선승 중 몇 판 이겼는지)
	// ═══════════════════════════════════════════
	UFUNCTION(BlueprintCallable, Category="OS|State")
	int32 GetTeamRoundWins(int32 TeamID) const;
	
	void AddRoundWin(int32 TeamID);
	
	// ═══════════════════════════════════════════
	// 라운드 번호
	// ═══════════════════════════════════════════
	UFUNCTION(BlueprintCallable, Category="OS|State")
	int32 GetCurrentRound() const { return CurrentRound; }
	
	void AdvanceRound();
	
	// ═══════════════════════════════════════════
	// 매치 승자
	// ═══════════════════════════════════════════
	UFUNCTION(BlueprintCallable, Category="OS|State")
	int32 GetMatchWinner() const { return MatchWinner; }
	
	void SetMatchWinner(int32 TeamID);
	
	// ═══════════════════════════════════════════
	// 델리게이트 (UI 바인딩용)
	// ═══════════════════════════════════════════
	UPROPERTY(BlueprintAssignable, Category="OS|Events")
	FOnMatchPhaseChanged OnMatchPhaseChanged;
	
	UPROPERTY(BlueprintAssignable, Category="OS|Events")
	FOnScoreChanged OnScoreChanged;
	
	UPROPERTY(BlueprintAssignable, Category="OS|Events")
	FOnMatchWinner OnMatchWinnerDeclared;
	
private:
	// ** Replicated 변수
	UPROPERTY(ReplicatedUsing = OnRep_MatchPhase)
	EOSMatchPhase MatchPhase = EOSMatchPhase::WaitingForPlayers;
	
	UPROPERTY(ReplicatedUsing = OnRep_Score)
	int32 TeamARoundScore = 0;
	
	UPROPERTY(ReplicatedUsing = OnRep_Score)
	int32 TeamBRoundScore = 0;
	
	UPROPERTY(Replicated)
	int32 TeamARoundWins = 0;
	
	UPROPERTY(Replicated)
	int32 TeamBRoundWins = 0;
	
	UPROPERTY(Replicated)
	int32 CurrentRound = 1;
	
	UPROPERTY(ReplicatedUsing = OnRep_MatchWinner)
	int32 MatchWinner = -1; // -1 : 미정
	
	// ** OnRep 콜백
	UFUNCTION()
	void OnRep_MatchPhase();
	
	UFUNCTION()
	void OnRep_Score();
	
	UFUNCTION()
	void OnRep_MatchWinner();
};
