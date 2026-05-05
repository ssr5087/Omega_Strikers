// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"

#include "OSGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class OMEGA_STRIKERS_API UOSGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	virtual void Init() override;

	// 세션 기능
	void CreateMySession(const FString& RoomName = TEXT(""));
	void FindMySession();

public:
	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	
	// 세션 결과를 UI에 뿌리기 위해
	TArray<FOnlineSessionSearchResult> CachedResults;

	// 델리게이트
	void OnCreateSessionComplete(FName SessionName, bool bSuccess);
	void OnDestroySessionComplete(FName SessionName, bool bSuccess);
	void OnFindSessionComplete(bool bSuccess);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void JoinSessionByIndex(int32 Index);
	FString BuildSessionDisplayText(int32 Index) const;
	FString BuildHostedSessionDisplayText() const;
	bool HasHostedSession() const;

public:
	void CreateSessionInternal();
	FString GetLocalPlayerNickname() const;

	bool bPendingCreateSession = false;
	FString PendingRoomName = TEXT("My Room");

	// UI 연결용
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSessionListUpdated);

	UPROPERTY(BlueprintAssignable)
	FOnSessionListUpdated OnSessionListUpdated;
};
