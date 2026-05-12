// Fill out your copyright notice in the Description page of Project Settings.
// CSCharSelectGameMode.h — 캐릭터 선택 전용 GameMode
// 천전천승 프로젝트
//
// L_CharSelect 레벨의 GameMode.
// 서버에서만 동작 — 중복 검증, 확정 관리, 전원 확정 시 아레나 이동

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
	
	// ══════════════════════════════════════
	//  캐릭터 선택 검증 (PlayerState RPC에서 호출)
	// ══════════════════════════════════════
	
	// 선택 요청 - 중복이면 false
	bool TrySelectCharacter(AOSPlayerState* Player, FName CharacterID);
	
	// 확정 요청 - 잠금
	bool TryConfirmCharacter(AOSPlayerState* Player);
	
	// 확정 취소
	void CancelConfirmCharacter(AOSPlayerState* Player);
	
	// ★ 호스트가 게임 시작 버튼을 누를 때 호출
	void StartArenaTravel();
	
protected:
	virtual void PostLogin(APlayerController* NewPlayer) override;
	
	// 아레나 맵 경로
	UPROPERTY(EditDefaultsOnly, Category= "OS|Map")
	FString ArenaMapPath = TEXT("/Game/Maps/Arena");

	// ★ 추가: 아레나 GameMode 클래스
	UPROPERTY(EditDefaultsOnly, Category= "OS|Map")
	TSubclassOf<AGameModeBase> ArenaGameModeClass;
	
private:
	// 전원 확정 -> 아레나 이동
	void OnAllPlayersConfirmed();
};
