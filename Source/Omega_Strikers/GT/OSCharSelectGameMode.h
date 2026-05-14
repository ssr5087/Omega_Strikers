// Fill out your copyright notice in the Description page of Project Settings.
// CSCharSelectGameMode.h — 캐릭터 선택 전용 GameMode
// 천전천승 프로젝트
//
// L_CharSelect 레벨의 GameMode.
// 서버에서만 동작 — 중복 검증, 확정 관리, 전원 확정 시 아레나 이동
// =====================================================
// OSCharSelectGameMode.h — 팀 선택 기능 추가 버전
// =====================================================
// 추가된 함수:
//   TryChangeTeam()     — 팀 변경 검증
//   GetTeamCount()      — 특정 팀 인원 수
//   GetMaxPlayersPerTeam() — 팀당 최대 인원 getter
// =====================================================

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "OSCharSelectGameMode.generated.h"

class AOSPlayerState;

UCLASS()
class OMEGA_STRIKERS_API AOSCharSelectGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	AOSCharSelectGameMode();
	
	virtual void BeginPlay() override;
	
	// ══════════════════════════════════════
	//  캐릭터 선택 검증 (PlayerState RPC에서 호출)
	// ══════════════════════════════════════
	
	// 선택 요청 - 중복이면 false
	bool TrySelectCharacter(AOSPlayerState* Player, FName CharacterID);
	
	// 확정 요청 - 잠금
	bool TryConfirmCharacter(AOSPlayerState* Player);
	
	// 확정 취소
	void CancelConfirmCharacter(AOSPlayerState* Player);
	
	// 팀 변경 검증
	bool TryChangeTeam(AOSPlayerState* Player, int32 NewTeamID);

	// 팀 인원 수 (위젯에서 표시용)
	int32 GetTeamCount(int32 TeamID) const;
	
	// 호스트가 게임 시작 버튼을 누를 때 호출
	void StartArenaTravel();
	
protected:
	virtual void PostLogin(APlayerController* NewPlayer) override;
	
	// 아레나 맵 경로
	UPROPERTY(EditDefaultsOnly, Category= "OS|Map")
	FString ArenaMapPath = TEXT("/Game/Maps/Arena");

	// 아레나 GameMode 클래스
	UPROPERTY(EditDefaultsOnly, Category= "OS|Map")
	TSubclassOf<AGameModeBase> ArenaGameModeClass;
	
	// 팀당 최대 인원 (에디터에서 조절 가능, 기본 3)
	UPROPERTY(EditDefaultsOnly, Category= "OS|Team")
	int32 MaxPlayersPerTeam = 3;
	
private:
	// 전원 확정 -> 아레나 이동
	void OnAllPlayersConfirmed();
	
	// 접속 시 인원 적은 팀에 자동 배정
	void AssignDefaultTeam(AOSPlayerState* PS);
};
