// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"

#include "OSGameInstance.generated.h"

USTRUCT(BlueprintType)
struct FSessionInfo
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly)
	FString roomName;
	
	UPROPERTY(BlueprintReadOnly)
	FString hostName;
	
	UPROPERTY(BlueprintReadOnly)
	FString playerCount;
	
	UPROPERTY(BlueprintReadOnly)
	int32 pingSpeed;
	
	UPROPERTY(BlueprintReadOnly)
	int32 index;
	
	inline FString ToString()
	{
		return FString::Printf(TEXT("[%d]%s : %s - %s, %dms"), index, *roomName, *hostName,  *playerCount, pingSpeed);
	}
};

// 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSearchSignature, const FSessionInfo&, SessionInfo);

// 세션 검색 상태 델리게이트 추가
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSearchStateSignature, bool, bIsSearching);

UCLASS()
class OMEGA_STRIKERS_API UOSGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	virtual void Init() override;

public:
	IOnlineSessionPtr sessionInterface;
	
	void CreateSession(FString roomName, int32 playerCount);
	
	// 세션 호스트 이름
	FString mySessionName = "SSR";
	
	UFUNCTION()
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	
	// 세션 생성 대기용 (ServerTravel 후 생성하기 위해)
	FString PendingRoomName;
	int32 PendingPlayerCount = 0;
	bool bPendingSessionCreate = false;

	// ServerTravel 먼저 하고, 맵 로드 후 세션 생성
	void HostAndCreateSession(FString roomName, int32 playerCount);

	// 맵 로드 완료 후 호출
	void CreatePendingSession();
	
	// 맵 로드 완료 콜백
	void OnPostLoadMap(UWorld* LoadedWorld);
	
public:
	// ------------------- 방 검색 --------------------------
	TSharedPtr<FOnlineSessionSearch> sessionSearch;
	void FindOtherSession();
	
	UFUNCTION()
	void OnFindSesssionsComplete(bool bWasSuccessful);
	
	// Session(방) 입장 함수
	void JoinSelectedSession(int32 index);
	
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type result);
	
	// 세션 나가기 (호스트면 파괴, 클라이언트면 떠나기) → 메인메뉴로 이동
	UFUNCTION(BlueprintCallable, Category = "OS|Session")
	void LeaveSession();
	
	void OnLeaveSessionDestroyComplete(FName SessionName, bool bSuccess);
	
	void ReturnToMainMenu();
	
	// 게임 시작
	void GameToStart();
	
	// 방 찾기 완료 콜백을 등록할 델리게이트
	FSearchSignature onSearchCompleted;
	
	// 세션 검색 상태 델리게이트 추가
	FSearchStateSignature onSearchState;

	// ═══════════════════════════════════════════════════════
	// 캐릭터 선택 저장 (ServerTravel 시 PlayerState 소멸 대비)
	// ═══════════════════════════════════════════════════════

	/** 캐릭터 선택 저장 */
	void SaveCharacterSelection(const FString& PlayerKey, FName CharacterID);

	/** 저장된 캐릭터 ID 조회 (없으면 NAME_None) */
	FName GetCharacterSelection(const FString& PlayerKey) const;

	/** 전체 선택 초기화 (새 매치 시작 시) */
	void ClearCharacterSelections();

	/** 전체 선택 맵 (디버그/로그용) */
	const TMap<FString, FName>& GetAllSelections() const { return CharacterSelections; }

	/** PlayerState에서 고유 키 추출 헬퍼 */
	static FString GetPlayerKey(const APlayerState* PS);
	
	// ═══════════════════════════════════════════════════════
	//  ★ 팀 배정 저장 (Seamless Travel 시 PlayerState 소멸 대비)
	// ═══════════════════════════════════════════════════════
	void SaveTeamAssignment(const FString& PlayerKey, int32 TeamID);
	int32 GetTeamAssignment(const FString& PlayerKey) const;
	void ClearTeamAssignments();
	
private:
	UPROPERTY()
	TMap<FString, FName> CharacterSelections;
	
	UPROPERTY()
	TMap<FString, int32> TeamAssignments;
};
