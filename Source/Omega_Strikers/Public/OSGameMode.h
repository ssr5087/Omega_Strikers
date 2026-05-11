// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Omega_Strikers/SSR/EXPOrb.h"
#include "Omega_Strikers/SSR/EXPSpawnPoint.h"
#include "OSGameMode.generated.h"

class APlayerBase;
// 매치 진행 단계
UENUM(BlueprintType)
enum class EOSMatchPhase : uint8
{
	WaitingForPlayers  UMETA(DisplayName = "Waiting"),
	Countdown          UMETA(DisplayName = "Countdown"),
	InProgress         UMETA(DisplayName = "InProgress"),
	RoundEnd           UMETA(DisplayName = "RoundEnd"),
	MatchEnd           UMETA(DisplayName = "MatchEnd"),
};

UCLASS()
class OMEGA_STRIKERS_API AOSGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	AOSGameMode();
	
	// ═══════════════════════════════════════════
	// 게임 설정
	// ═══════════════════════════════════════════
	UPROPERTY(EditAnywhere, Category="OS|Match")
	int32 PlayersPerTeam = 3; // 3vs3
	
	UPROPERTY(EditAnywhere, Category="OS|Match")
	int32 ScoresToWinRound = 3; // 3골 -> 라운드 승리
	
	UPROPERTY(EditAnywhere, Category="OS|Match")
	int32 RoundsToWinMatch = 2; // 3판2선승
	
	UPROPERTY(EditAnywhere, Category="OS|Match")
	float CountdownDuration = 3.0f;
	
	UPROPERTY(EditAnywhere, Category="OS|Match")
	float RoundEndDelay = 3.0f; // 라운드 종료 후 딜레이

	// ═══════════════════════════════════════════
	// ★ 캐릭터 → Pawn 매핑
	// ═══════════════════════════════════════════
	
	/**
	 * CharacterID → Pawn 클래스 매핑
	 * 에디터 Details에서 설정:
	 *   "Aimi"    → BP_Aimi
	 *   "Zentaro" → BP_Zentaro
	 *   ...
	 */
	UPROPERTY(EditAnywhere, Category="OS|Character")
	TMap<FName, TSubclassOf<APlayerBase>> CharacterPawnMap;
	
	// ═══════════════════════════════════════════
	// 플레이어 관리 오버라이드
	// ═══════════════════════════════════════════
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	/** ★ 선택된 캐릭터에 맞는 Pawn 클래스 반환 */
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	
	// ═══════════════════════════════════════════
	// 매치 흐름
	// ═══════════════════════════════════════════
	UFUNCTION(BlueprintCallable, Category="OS|Match")
	void TryStartMatch();
	
	UFUNCTION(BlueprintCallable, Category="OS|Match")
	void StartCountdown();
	
	UFUNCTION(BlueprintCallable, Category="OS|Match")
	void StartRound();
	
	UFUNCTION(BlueprintCallable, Category="OS|Match")
	void OnGoalScored(int32 ScoringTeam);
	
	UFUNCTION(BlueprintCallable, Category="OS|Match")
	void EndRound(int32 WinningTeam);
	
	UFUNCTION(BlueprintCallable, Category="OS|Match")
	void EndMatch(int32 WinningTeam);
	
	// ═══════════════════════════════════════════
	// 상태 조회
	// ═══════════════════════════════════════════
	UFUNCTION(BlueprintCallable, Category="OS|Match")
	EOSMatchPhase GetMatchPhase() const;
	
	UFUNCTION(BlueprintCallable, Category="OS|Match")
	bool IsReadyToStart() const;
	
	// ═══════════════════════════════════════════
	// Core 스폰 설정
	// ═══════════════════════════════════════════
	UPROPERTY(EditAnywhere, Category="OS|Core")
	TSubclassOf<class ACoreBall> CoreBallClass;
	
	// 수동 스폰 위치 (CoreArena가 없을 경우)
	UPROPERTY(EditAnywhere, Category="OS|Core")
	FVector CoreSpawnLocation = FVector::ZeroVector;
	
	// 코어 스폰 Z 오프셋 (바닥 위로 띄우기)
	UPROPERTY(EditAnywhere, Category="OS|Core")
	float CoreSpawnZOffset = 150.f;
	
	// ═══════════════════════════════════════════
	// 아레나 레퍼런스
	// ═══════════════════════════════════════════
	UPROPERTY(VisibleAnywhere, Category="OS|Arena")
	class ACoreArena* ArenaRef = nullptr;
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY()
	ACoreBall* ActiveCoreBall = nullptr;
	
public:
	// 팀 배정
	int32 AssignTeam(APlayerController* NewPlayer);
	int32 GetTeamPlayerCount(int32 TeamID) const;
	void SpawnCoreBall();
	
	// 타이머 핸들
	FTimerHandle CountdownTimer;
	FTimerHandle RoundEndTimer;

public:
	// 경험치오브 스폰액터 관리
	UPROPERTY(EditAnywhere)
	TSubclassOf<AEXPOrb> EXPOrbClass;
	
	UPROPERTY()
	TArray<AEXPSpawnPoint*> SpawnPoints;
	
	FTimerHandle SpawnTimer;

	UFUNCTION()
	void SpawnEXPOrbs();
	
	UFUNCTION()
	void SpawnAllEXPOrbs();
	
	UFUNCTION()
	void OnOrbDestroyed(AActor* DestroyedActor);
	
	// 시연용 임시 코드(알파 이후 삭제)
	UPROPERTY(EditDefaultsOnly, Category="Temp")
	TSubclassOf<APawn> ServerPawnClass;

	UPROPERTY(EditDefaultsOnly, Category="Temp")
	TSubclassOf<APawn> ClientPawnClass;
};
