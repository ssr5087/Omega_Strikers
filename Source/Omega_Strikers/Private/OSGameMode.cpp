// Fill out your copyright notice in the Description page of Project Settings.
// 서버 전용 게임 로직 구현

#include "OSGameMode.h"

#include "OSGameState.h"
#include "OSPlayerState.h"
#include "Core/CoreArena.h"
#include "Core/CoreBall.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Omega_Strikers/Omega_Strikers.h"
#include "OSTopDownController.h"
#include "PlayerBase.h"

AOSGameMode::AOSGameMode()
{
	// ** 기본 클래스 지정
	GameStateClass = AOSGameState::StaticClass();
	PlayerStateClass = AOSPlayerState::StaticClass();
	PlayerControllerClass = AOSTopDownController::StaticClass();
	
	// 심리스 트래블 지원
	bUseSeamlessTravel = true;
}

void AOSGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	// 레벨에 배치된 CoreArena 찾기
	TArray<AActor*> foundArenas;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACoreArena::StaticClass(), foundArenas);

	if (foundArenas.Num() > 0)
	{
		ArenaRef = Cast<ACoreArena>(foundArenas[0]);
		LOG_GT(TEXT("CoreArena found at (%.0f, %.0f, %.0f)"),
			ArenaRef->GetActorLocation().X,
			ArenaRef->GetActorLocation().Y,
			ArenaRef->GetActorLocation().Z);
	}
	else
	{
		LOG_GT_W(TEXT("CoreArena not found! CoreBall will spawn at CoreSpawnLocation"));
	}
	
	LOG_GT(TEXT("플레이어 대기중"))
}

void AOSGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	if (!NewPlayer) return;
	
	// ** 팀 배정
	int32 AssignedTeam = AssignTeam(NewPlayer);
	AOSPlayerState* ps = NewPlayer->GetPlayerState<AOSPlayerState>();
	
	if (ps)
	{
		ps->SetTeamID(AssignedTeam);
		LOG_GT(TEXT("Player %s 소속 팀 : %d"), *ps->GetPlayerName(), AssignedTeam);
	}
	
	// ** 인원 체크 -> 자동 시작
	int32 totalPlayers = GetTeamPlayerCount(0) + GetTeamPlayerCount(1);
	LOG_GT(TEXT("전체 플레이어 : %d / %d"), totalPlayers, PlayersPerTeam * 2);
	
	if (IsReadyToStart()) TryStartMatch();
}

void AOSGameMode::Logout(AController* Exiting)
{
	if (APlayerController* pc = Cast<APlayerController>(Exiting))
	{
		AOSPlayerState* ps = pc->GetPlayerState<AOSPlayerState>();
		if (ps)
		{
			LOG_GT(TEXT("Player %s (Team %d) 탈퇴"), *ps->GetPlayerName(), ps->GetTeamID());
		}
	}
	
	Super::Logout(Exiting);
}

// ═══════════════════════════════════════════════════════
// 팀 배정 (인원 적은 팀에 자동 배정)
// ═══════════════════════════════════════════════════════
int32 AOSGameMode::AssignTeam(APlayerController* NewPlayer)
{
	int32 teamACount = GetTeamPlayerCount(0);
	int32 teamBCount = GetTeamPlayerCount(1);
	
	// 인원 적은 팀에 배정, 같으면 A팀
	return (teamACount <= teamBCount) ? 0 : 1;
}

int32 AOSGameMode::GetTeamPlayerCount(int32 TeamID) const
{
	int32 count = 0;
	
	AOSGameState* gs = GetGameState<AOSGameState>();
	if (!gs) return 0;
	
	for (APlayerState* basePS : gs->PlayerArray)
	{
		AOSPlayerState* ps = Cast<AOSPlayerState>(basePS);
		if (ps && ps->GetTeamID() == TeamID) count++;
	}
	
	return count;
}

// ═══════════════════════════════════════════════════════
// 스폰 위치 선택 (팀별 PlayerStart)
// ═══════════════════════════════════════════════════════
AActor* AOSGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	AOSPlayerState* ps = Player ? Player->GetPlayerState<AOSPlayerState>() : nullptr;
	int32 teamID = ps ? ps->GetTeamID() : 0;
	
	// PlayerStart의 Tag로 팀 구분
	// Tag TeamA (0) -> Red, TeamB (1) -> Blue
	FString desiredTag = FString::Printf(TEXT("Team%d"), teamID);
	
	TArray<AActor*> starts;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), starts);
	
	// 태그 매칭되는 것 중 랜덤 선택
	TArray<AActor*> teamStarts;
	for (AActor* start : starts)
	{
		if (start->ActorHasTag(FName(*desiredTag))) teamStarts.Add(start);
	}
	
	if (teamStarts.Num() > 0) return teamStarts[FMath::RandRange(0, teamStarts.Num() - 1)];
	
	// 매칭 안되면 기본
	return Super::ChoosePlayerStart_Implementation(Player);
}

// ═══════════════════════════════════════════════════════
// 매치 흐름
// ═══════════════════════════════════════════════════════
bool AOSGameMode::IsReadyToStart() const
{
	int32 teamA = GetTeamPlayerCount(0);
	int32 teamB = GetTeamPlayerCount(1);
	return teamA >= PlayersPerTeam && teamB >= PlayersPerTeam;
}

EOSMatchPhase AOSGameMode::GetMatchPhase() const
{
	AOSGameState* gs = GetGameState<AOSGameState>();
	return gs ? gs->GetMatchPhase() : EOSMatchPhase::WaitingForPlayers;
}

void AOSGameMode::TryStartMatch()
{
	if (!IsReadyToStart()) return;
	
	AOSGameState* gs = GetGameState<AOSGameState>();
	if (!gs) return;
	if (gs->GetMatchPhase() != EOSMatchPhase::WaitingForPlayers) return;
	
	LOG_GT(TEXT("전체 플레이어 준비 완료: 스타팅 카운트다운"));
	StartCountdown();
}

void AOSGameMode::StartCountdown()
{
	AOSGameState* gs = GetGameState<AOSGameState>();
	if (!gs) return;
	
	gs->SetMatchPhase(EOSMatchPhase::Countdown);
	
	// 카운트 다운 후 라운드 시작
	GetWorldTimerManager().SetTimer(
		CountdownTimer, this, &AOSGameMode::StartRound, CountdownDuration, false);
}

void AOSGameMode::StartRound()
{
	AOSGameState* gs = GetGameState<AOSGameState>();
	if (!gs) return;
	
	gs->SetMatchPhase(EOSMatchPhase::InProgress);
	gs->ResetRoundScores();
	
	LOG_GT(TEXT("Round %d Started!"), gs->GetCurrentRound());
	
	// Core 스폰 또는 리셋
	if (!ActiveCoreBall)
	{
		SpawnCoreBall();
	}
	else
	{
		// 아레나 중앙으로 리셋
		FVector resetLocation = CoreSpawnLocation;
		if (ArenaRef)
		{
			resetLocation = ArenaRef->GetActorLocation();
			resetLocation.Z += CoreSpawnZOffset;
		}
		ActiveCoreBall->SetActorLocation(resetLocation);
		//ActiveCoreBall->ResetVelocity();
		LOG_GT(TEXT("CoreBall reset to center (%.0f, %.0f, %.0f)"),
			resetLocation.X, resetLocation.Y, resetLocation.Z);
	}
}

void AOSGameMode::SpawnCoreBall()
{
	if (!CoreBallClass)
	{
		// 클래스 미지정 시 기본 ACoreBall 사용
		CoreBallClass = ACoreBall::StaticClass();
	}
	
	// 스폰 위치: CoreArena 중앙 또는 수동 지정 위치
	FVector spawnLocation = CoreSpawnLocation;
	
	if (ArenaRef)
	{
		spawnLocation = ArenaRef->GetActorLocation();
		spawnLocation.Z += CoreSpawnZOffset;
		LOG_GT(TEXT("CoreBall spawning at Arena center"));
	}
	
	FActorSpawnParameters spawnParams;
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	ActiveCoreBall = GetWorld()->SpawnActor<ACoreBall>(
		CoreBallClass, spawnLocation, FRotator::ZeroRotator, spawnParams);
	
	if (ActiveCoreBall)
	{
		// 골 이벤트 바인딩
		ActiveCoreBall->OnGoalScored.AddDynamic(this, &AOSGameMode::OnGoalScored);

		// ★ 모든 플레이어에게 CoreBall 등록
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			if (APlayerBase* Player = Cast<APlayerBase>(It->Get()->GetPawn()))
			{
				Player->CachedCoreBall = ActiveCoreBall;
			}
		}

		LOG_GT(TEXT("CoreBall spawned at (%.0f, %.0f, %.0f)"),
			spawnLocation.X, spawnLocation.Y, spawnLocation.Z);
	}
}

void AOSGameMode::OnGoalScored(int32 ScoringTeam)
{
	AOSGameState* gs = GetGameState<AOSGameState>();
	if (!gs) return;

	if (gs->GetMatchPhase() != EOSMatchPhase::InProgress) return;

	// 점수 추가
	gs->AddScore(ScoringTeam);

	int32 score = gs->GetTeamRoundScore(ScoringTeam);
	LOG_GT(TEXT("Team %d scored! (%d / %d)"), ScoringTeam, score, ScoresToWinRound);

	// 라운드 승리 체크
	if (score >= ScoresToWinRound)
	{
		EndRound(ScoringTeam);
	}
	else
	{
		// TODO: Core 리셋 후 이어서 진행 (3초 후 - CoreBall 내부 타이머)
	}
}

void AOSGameMode::EndRound(int32 WinningTeam)
{
	AOSGameState* gs = GetGameState<AOSGameState>();
	if (!gs) return;

	gs->SetMatchPhase(EOSMatchPhase::RoundEnd);
	gs->AddRoundWin(WinningTeam);

	int32 roundWins = gs->GetTeamRoundWins(WinningTeam);
	LOG_GT(TEXT("Team %d wins round! (%d / %d rounds)"), WinningTeam, roundWins, RoundsToWinMatch);

	// 매치 승리 체크
	if (roundWins >= RoundsToWinMatch)
	{
		EndMatch(WinningTeam);
		return;
	}

	// 다음 라운드 준비
	gs->AdvanceRound();

	GetWorldTimerManager().SetTimer(
		RoundEndTimer, this, &AOSGameMode::StartCountdown, RoundEndDelay, false);
}

void AOSGameMode::EndMatch(int32 WinningTeam)
{
	AOSGameState* gs = GetGameState<AOSGameState>();
	if (!gs) return;

	gs->SetMatchPhase(EOSMatchPhase::MatchEnd);
	gs->SetMatchWinner(WinningTeam);

	LOG_GT(TEXT("==MATCH OVER== Team %d WINS!"), WinningTeam);

	// TODO: 결과 화면 -> 일정 시간 후 로비로 복귀
}
