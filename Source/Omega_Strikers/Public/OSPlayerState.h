// Fill out your copyright notice in the Description page of Project Settings.
// 플레이어별 상태 ( 모든 클라이언트에 리플리케이트 )
// 팀, 선택한 캐릭터, 개인 스탯 등

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "OSPlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTeamAssigned, AOSPlayerState*, Player, int32, TeamID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSelectRejected, FName, CharacterID, const FString&, Reason);

UCLASS()
class OMEGA_STRIKERS_API AOSPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	AOSPlayerState();
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	// ═══════════════════════════════════════════
	// 팀
	// ═══════════════════════════════════════════
	UFUNCTION(BlueprintCallable, Category="OS|Player")
	int32 GetTeamID() const { return TeamID; }
	
	void SetTeamID(int32 NewTeam);
	
	// ═══════════════════════════════════════════
	// 캐릭터 선택 (아이미, 젠타로, 나오, 에스텔, 루나 등)
	// ═══════════════════════════════════════════
	UFUNCTION(BlueprintCallable, Category="OS|Player")
	FName  GetSelectedCharacter() const { return SelectedCharacter; }
	
	UFUNCTION(Server, Reliable)
	void Server_SelectCharacter(FName CharacterID);
	
	// ═══════════════════════════════════════════
	// 캐릭터 선택 — 네트워크 RPC
	// ═══════════════════════════════════════════
	UFUNCTION(BlueprintCallable, Category = "OS|CharSelect")
	bool IsCharacterConfirmed() const { return bCharacterConfirmed; }

	/** 캐릭터 선택 요청 (GameMode에서 중복 검증) */
	UFUNCTION(Server, Reliable)
	void Server_RequestSelectCharacter(FName CharacterID);

	/** 캐릭터 확정 요청 */
	UFUNCTION(Server, Reliable)
	void Server_RequestConfirmCharacter();

	/** 확정 취소 */
	UFUNCTION(Server, Reliable)
	void Server_RequestCancelConfirm();

	/** 선택 거부 알림 (서버 → 클라이언트) */
	UFUNCTION(Client, Reliable)
	void Client_OnSelectRejected(FName CharacterID, const FString& Reason);

	/** 위젯에서 바인딩 — 거부 시 UI 갱신 */
	UPROPERTY(BlueprintAssignable, Category = "OS|Events")
	FOnSelectRejected OnSelectRejected;
	
	// ═══════════════════════════════════════════
	// 인게임 스탯 (현재 매치)
	// ═══════════════════════════════════════════
	UFUNCTION(BlueprintCallable, Category="OS|Player")
	int32 GetGoals() const { return Goals; }
	
	UFUNCTION(BlueprintCallable, Category="OS|Player")
	int32 GetAssists() const { return Assists; }
	
	UFUNCTION(BlueprintCallable, Category="OS|Player")
	int32 GetKnockouts() const { return Knockouts; }
	
	// 서버 전용 Setter
	void AddGoal();
	void AddAssist();
	void AddKnockout();
	void ResetMatchStats();
	
	// ═══════════════════════════════════════════
	// 준비 상태 (로비용)
	// ═══════════════════════════════════════════
	UFUNCTION(BlueprintCallable, Category="OS|Player")
	bool IsReady() const { return bIsReady; }
	
	UFUNCTION(Server, Reliable)
	void Server_SetReady(bool bReady);
	
	// ═══════════════════════════════════════════
	// 델리게이트
	// ═══════════════════════════════════════════
	UPROPERTY(BlueprintAssignable, Category="OS|Events")
	FOnTeamAssigned OnTeamAssigned;
	
private:
	UPROPERTY(ReplicatedUsing = OnRep_TeamID)
	int32 TeamID = -1; // -1 : 미배정
	
	UPROPERTY(Replicated)
	FName SelectedCharacter = NAME_None;
	
	UPROPERTY(Replicated)
	int32 Goals = 0;
	
	UPROPERTY(Replicated)
	int32 Assists = 0;
	
	UPROPERTY(Replicated)
	int32 Knockouts = 0;
	
	UPROPERTY(Replicated)
	bool bIsReady = false;
	
	// OnRep
	UFUNCTION()
	void OnRep_TeamID();

	UPROPERTY(Replicated)
	bool bCharacterConfirmed = false;
};
