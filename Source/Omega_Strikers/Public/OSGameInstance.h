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
	
};
