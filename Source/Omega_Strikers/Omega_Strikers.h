// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogTeam, Log, All);

// =====================
// GT (문경태)
// =====================
#define LOG_GT(Format, ...) \
UE_LOG(LogTeam, Log, TEXT("[GT][%s:%d] " Format), TEXT(__FUNCTION__), __LINE__, ##__VA_ARGS__)

#define LOG_GT_W(Format, ...) \
UE_LOG(LogTeam, Warning, TEXT("[GT][W][%s:%d] " Format), TEXT(__FUNCTION__), __LINE__, ##__VA_ARGS__)

#define LOG_GT_E(Format, ...) \
UE_LOG(LogTeam, Error, TEXT("[GT][E][%s:%d] " Format), TEXT(__FUNCTION__), __LINE__, ##__VA_ARGS__)


// =====================
// SR (서승률)
// =====================
#define LOG_SR(Format, ...) \
UE_LOG(LogTeam, Log, TEXT("[SR][%s:%d] " Format), TEXT(__FUNCTION__), __LINE__, ##__VA_ARGS__)

#define LOG_SR_W(Format, ...) \
UE_LOG(LogTeam, Warning, TEXT("[SR][W][%s:%d] " Format), TEXT(__FUNCTION__), __LINE__, ##__VA_ARGS__)

#define LOG_SR_E(Format, ...) \
UE_LOG(LogTeam, Error, TEXT("[SR][E][%s:%d] " Format), TEXT(__FUNCTION__), __LINE__, ##__VA_ARGS__)


// =====================
// SM (천성민)
// =====================
#define LOG_SM(Format, ...) \
UE_LOG(LogTeam, Log, TEXT("[SM][%s:%d] " Format), TEXT(__FUNCTION__), __LINE__, ##__VA_ARGS__)

#define LOG_SM_W(Format, ...) \
UE_LOG(LogTeam, Warning, TEXT("[SM][W][%s:%d] " Format), TEXT(__FUNCTION__), __LINE__, ##__VA_ARGS__)

#define LOG_SM_E(Format, ...) \
UE_LOG(LogTeam, Error, TEXT("[SM][E][%s:%d] " Format), TEXT(__FUNCTION__), __LINE__, ##__VA_ARGS__)