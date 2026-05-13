// Fill out your copyright notice in the Description page of Project Settings.
// 서버 전용 게임 로직 구현

#include "OSGameMode.h"

#include "OSGameInstance.h"
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
	
	// ★ 기본 폰도 PlayerBase로 — DefaultPawn 스폰 방지
	DefaultPawnClass = APlayerBase::StaticClass();
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
	
	
	// 경험치오브 스폰액터 관리
	
	// 서버에서 처리
	// if (!HasAuthority())
	// 	return;
	
	TArray<AActor*> FoundActors;
	
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEXPSpawnPoint::StaticClass(), FoundActors);
	
	for (AActor* Actor : FoundActors)
	{
		AEXPSpawnPoint* SpawnPoint = Cast<AEXPSpawnPoint>(Actor);

		if (SpawnPoint)
		{
			SpawnPoints.Add(SpawnPoint);
		}
	}
	
	// 시작 즉시 한번 생성
	SpawnAllEXPOrbs();
	
	GetWorldTimerManager().SetTimer(
		SpawnTimer,
		this,
		&AOSGameMode::SpawnEXPOrbs,
		10.f,
		true
	);
}

// ═══════════════════════════════════════════════════════
// ★ 선택된 캐릭터에 맞는 Pawn 클래스 반환
// ═══════════════════════════════════════════════════════
UClass* AOSGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	if (!InController) return DefaultPawnClass;
	
	// 1) GameInstance에서 선택된 캐릭터 ID 가져오기
	UOSGameInstance* gi = Cast<UOSGameInstance>(GetGameInstance());
	if ( !gi )
	{
		LOG_GT_E(TEXT("GameInstance 캐스팅 실패 → DefaultPawn"));
		return DefaultPawnClass;
	}
	
	// PlayerState에서 UniqueNetId 기반 키 추출
	APlayerController* pc = Cast<APlayerController>(InController);
	if ( !pc || !pc->PlayerState )
	{
		LOG_GT_E(TEXT("PC 또는 PlayerState 없음 → DefaultPawn"));
		return DefaultPawnClass;
	}
	
	const FString playerKey = UOSGameInstance::GetPlayerKey(pc->PlayerState);
	const FName characterID = gi->GetCharacterSelection(playerKey);
	
	if ( characterID.IsNone() )
	{
		LOG_GT_E(TEXT("[%s] 캐릭터 미선택 → DefaultPawn"), *playerKey);
		
		// ★ 디버그: 저장된 전체 목록 출력
		for (const auto& pair : gi->GetAllSelections())
		{
			LOG_GT(TEXT("  저장된 키: [%s] → %s"), *pair.Key, *pair.Value.ToString());
		}
		return DefaultPawnClass;
	}
	
	// 2) CharacterPawnMap에서 PawnClass 조회
	TSubclassOf<APlayerBase>* foundClass = CharacterPawnMap.Find(characterID);
	if ( foundClass && *foundClass )
	{
		LOG_GT(TEXT("[%s] → %s → %s"),
			*playerKey,
			*characterID.ToString(),
			*(*foundClass)->GetName());
		return *foundClass;
	}
	
	// 3) 매핑 없음 → DefaultPawn
	LOG_GT_E(TEXT("[%s]: '%s' CharacterPawnMap에 매핑 없음 → DefaultPawn"),
		*playerKey, *characterID.ToString());
	return DefaultPawnClass;
}

void AOSGameMode::PostLogin(APlayerController* NewPlayer)
{
	if (!NewPlayer) return;
	
	// ** 팀 배정
	int32 assignedTeam = AssignTeam(NewPlayer);
	AOSPlayerState* ps = NewPlayer->GetPlayerState<AOSPlayerState>();
	
	if ( ps )
	{
		ps->SetTeamID(assignedTeam);
		LOG_GT(TEXT("Player %s 소속 팀 : %d"), *ps->GetPlayerName(), assignedTeam);
	}

	// ★ GameInstance에서 선택 캐릭터 로그
	UOSGameInstance* gi = Cast<UOSGameInstance>(GetGameInstance());
	if ( gi && ps )
	{
		FString playerKey = UOSGameInstance::GetPlayerKey(ps);
		FName CharID = gi->GetCharacterSelection(playerKey);
		LOG_GT(TEXT("Player %s [%s] 선택 캐릭터: %s"),
			*NewPlayer->GetName(),
			*playerKey,
			CharID.IsNone() ? TEXT("(없음)") : *CharID.ToString());
	}

	Super::PostLogin(NewPlayer);
	
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
	
	GetWorldTimerManager().ClearTimer(RoundEndTimer);
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
	gs->ClearGoalSequence();
	bGoalScoredThisSequence = false;
	
	LOG_GT(TEXT("Match started or resumed after goal"));
	
	// Core 스폰 또는 리셋
	if (!ActiveCoreBall)
	{
		SpawnCoreBall();
	}
	else
	{
		FVector resetLocation = CoreSpawnLocation;
		if (ArenaRef)
		{
			resetLocation = ArenaRef->GetActorLocation();
			resetLocation.Z += CoreSpawnZOffset;
		}
		ActiveCoreBall->SetHomeLocation(resetLocation);
		ActiveCoreBall->ResetToCenter();
		if (ArenaRef)
		{
			ArenaRef->ResetForNewRound();
		}
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
		ActiveCoreBall->SetHomeLocation(spawnLocation);

		// 골 이벤트 바인딩
		ActiveCoreBall->OnGoalScored.Clear();
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
	if (gs->IsGoalSequenceActive()) return;

	// 점수 추가
	if (bGoalScoredThisSequence) return;
	bGoalScoredThisSequence = true;
	gs->StartGoalSequence(ScoringTeam, GetWorld()->GetTimeSeconds() + RoundEndDelay);
	gs->AddScore(ScoringTeam);

	if (IsValid(ActiveCoreBall))
	{
		ActiveCoreBall->Destroy();
		ActiveCoreBall = nullptr;
	}

	int32 score = gs->GetTeamRoundScore(ScoringTeam);
	LOG_SR_W(TEXT("Team %d scored! (%d / %d)"), ScoringTeam, score, ScoresToWinRound);

	if (score >= ScoresToWinRound)
	{
		GetWorldTimerManager().ClearTimer(RoundEndTimer);
		EndMatch(ScoringTeam);
	}
	else
	{
		gs->SetMatchPhase(EOSMatchPhase::RoundEnd);
		GetWorldTimerManager().SetTimer(
			RoundEndTimer, this, &AOSGameMode::RestartPlayAfterGoal, RoundEndDelay, false);
	}
}

void AOSGameMode::RestartPlayAfterGoal()
{
	AOSGameState* gs = GetGameState<AOSGameState>();
	if (!gs) return;

	if (gs->GetMatchPhase() == EOSMatchPhase::MatchEnd)
	{
		return;
	}

	bGoalScoredThisSequence = false;
	StartRound();
}

void AOSGameMode::EndRound(int32 WinningTeam)
{
	EndMatch(WinningTeam);
}

void AOSGameMode::EndMatch(int32 WinningTeam)
{
	AOSGameState* gs = GetGameState<AOSGameState>();
	if (!gs) return;

	GetWorldTimerManager().ClearTimer(CountdownTimer);
	GetWorldTimerManager().ClearTimer(RoundEndTimer);
	bGoalScoredThisSequence = false;
	gs->SetMatchPhase(EOSMatchPhase::MatchEnd);
	gs->SetMatchWinner(WinningTeam);

	LOG_GT(TEXT("==MATCH OVER== Team %d WINS!"), WinningTeam);

	// TODO: 결과 화면 -> 일정 시간 후 로비로 복귀
}

// 경험치 EXPOrb 스폰
void AOSGameMode::SpawnEXPOrbs()
{
	if (SpawnPoints.Num() < 3) return;

	// 랜덤 셔플
	TArray<AEXPSpawnPoint*> Shuffled = SpawnPoints;

	for (int32 i = 0; i < Shuffled.Num(); i++)
	{
		int32 RandIndex = FMath::RandRange(i, Shuffled.Num() - 1);
		Shuffled.Swap(i, RandIndex);
	}

	// 앞에서 3개 선택
	for (int32 i = 0; i < 3; i++)
	{
		AEXPSpawnPoint* Point = Shuffled[i];
		if (!Point || Point->bHasOrb) continue;

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AEXPOrb* Orb = GetWorld()->SpawnActor<AEXPOrb>(
			EXPOrbClass,
			Point->GetActorLocation(),
			FRotator::ZeroRotator,
			Params
		);

		if (Orb)
		{
			Point->bHasOrb = true;

			// 🔥 Orb가 사라질 때 다시 false
			Orb->OnDestroyed.AddDynamic(this, &AOSGameMode::OnOrbDestroyed);
		}
	}
}

void AOSGameMode::SpawnAllEXPOrbs()
{
	for (AEXPSpawnPoint* Point : SpawnPoints)
	{
		if (!Point) continue;

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride =
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AEXPOrb* Orb = GetWorld()->SpawnActor<AEXPOrb>(
			EXPOrbClass,
			Point->GetActorLocation(),
			FRotator::ZeroRotator,
			Params
		);

		if (Orb)
		{
			LOG_GT(TEXT("초기 Orb 생성 완료"));
		}
	}
}

void AOSGameMode::OnOrbDestroyed(AActor* DestroyedActor)
{
	AEXPOrb* Orb = Cast<AEXPOrb>(DestroyedActor);
	if (!Orb) return;

	for (AEXPSpawnPoint* Point : SpawnPoints)
	{
		if (Point && Point->GetActorLocation().Equals(Orb->GetActorLocation(), 1.f))
		{
			Point->bHasOrb = false;
			break;
		}
	}
}
