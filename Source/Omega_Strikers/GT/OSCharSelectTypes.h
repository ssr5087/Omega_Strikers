// Fill out your copyright notice in the Description page of Project Settings.
// CSCharSelectTypes.h — 캐릭터 선택 공용 구조체
// 천전천승 프로젝트

#pragma once

#include "CoreMinimal.h"
#include "OSCharSelectTypes.generated.h"

// ═══════════════════════════════════════════════════════
//  캐릭터 선택 엔트리 (GameState에서 리플리케이트)
// ═══════════════════════════════════════════════════════
USTRUCT(BlueprintType)
struct FOSCharSelectEntry
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly)
	int32 PlayerIndex = -1;
	
	UPROPERTY(BlueprintReadOnly)
	FString PlayerName;
	
	UPROPERTY(BlueprintReadOnly)
	FName CharacterID = NAME_None;
	
	UPROPERTY(BlueprintReadOnly)
	bool bConfirmed = false;
	
	UPROPERTY(BlueprintReadOnly)
	int32 TeamID = -1;
};

// 카드 상태
UENUM(BlueprintType)
enum class ECardState : uint8
{
	Available,
	SelectedByMe,
	ConfirmedByMe,
	LockedByOther
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCharSelectListUpdated);
