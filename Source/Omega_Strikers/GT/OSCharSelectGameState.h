// Fill out your copyright notice in the Description page of Project Settings.
// CSCharSelectGameState.h — 캐릭터 선택 GameState
// 천전천승 프로젝트
//
// 모든 클라이언트에 선택 목록을 리플리케이트

#pragma once

#include "CoreMinimal.h"
#include "OSCharSelectTypes.h"
#include "GameFramework/GameStateBase.h"
#include "OSCharSelectGameState.generated.h"

class AOSPlayerState;
struct FOSCharSelectEntry;

UCLASS()
class OMEGA_STRIKERS_API AOSCharSelectGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	AOSCharSelectGameState();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	// ═══════════════════════════════════════
	//  선택 목록 (리플리케이트)
	// ═══════════════════════════════════════
	
	UPROPERTY(ReplicatedUsing = OnRep_CharSelectList, BlueprintReadOnly, Category = "OS|CharSelect")
	TArray<FOSCharSelectEntry> CharSelectList;
	
	UPROPERTY(BlueprintAssignable, Category="OS|Events")
	FOnCharSelectListUpdated OnCharSelectListUpdated;
	
	// ═══════════════════════════════════════
	//  헬퍼 함수 (서버 전용)
	// ═══════════════════════════════════════
	
	FOSCharSelectEntry& FindOrAddEntry(AOSPlayerState* PS);
	bool IsCharacterLocked(FName CharacterID) const;
	FString GetLockerName(FName CharacterID) const;
	bool AreAllPlayersConfirmed() const;
	void BroadcastCharSelectUpdate();
	
	// ═══════════════════════════════════════
	//  필요 인원 (리플리케이트)
	// ═══════════════════════════════════════

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "OS|CharSelect")
	int32 RequiredPlayerCount = 2;
	
private:
	UFUNCTION()
	void OnRep_CharSelectList();
};
