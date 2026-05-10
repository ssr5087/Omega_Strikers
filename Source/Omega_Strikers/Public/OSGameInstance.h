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
	
public:
	// ------------------- 방 검색 --------------------------
	TSharedPtr<FOnlineSessionSearch> sessionSearch;
	void FindOtherSession();
	
	UFUNCTION()
	void OnFindSesssionsComplete(bool bWasSuccessful);
	
	// Session(방) 입장 함수
	void JoinSelectedSession(int32 index);
	
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type result);
	
	// 게임 시작
	void GameToStart();
	
	// 방 찾기 완료 콜백을 등록할 델리게이트
	FSearchSignature onSearchCompleted;
	
	// 세션 검색 상태 델리게이트 추가
	FSearchStateSignature onSearchState;

	// ═══════════════════════════════════════════════════════
	//  ★ 캐릭터 선택 저장 (ServerTravel 시 PlayerState 소멸 대비)
	//  Key: PlayerIndex (0, 1, 2, ...)
	//  Value: CharacterID ("Aimi", "Zentaro", ...)
	// ═══════════════════════════════════════════════════════

	/** 캐릭터 선택 저장 */
	void SaveCharacterSelection(int32 PlayerIndex, FName CharacterID);

	/** 저장된 캐릭터 ID 조회 (없으면 NAME_None) */
	FName GetCharacterSelection(int32 PlayerIndex) const;

	/** 전체 선택 초기화 (새 매치 시작 시) */
	void ClearCharacterSelections();

	/** 전체 선택 맵 (디버그/로그용) */
	const TMap<int32, FName>& GetAllSelections() const { return CharacterSelections; }

private:
	UPROPERTY()
	TMap<int32, FName> CharacterSelections;
};
