// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogTeam, Log, All);

// =====================
// GT (팀원1)
// =====================
#define LOG_GT(Format, ...) \
UE_LOG(LogTeam, Log, TEXT("[GT] " Format), ##__VA_ARGS__)

#define LOG_GT_W(Format, ...) \
UE_LOG(LogTeam, Warning, TEXT("[GT][W] " Format), ##__VA_ARGS__)

#define LOG_GT_E(Format, ...) \
UE_LOG(LogTeam, Error, TEXT("[GT][E] " Format), ##__VA_ARGS__)


// =====================
// SR (팀원2)
// =====================
#define LOG_SR(Format, ...) \
UE_LOG(LogTeam, Log, TEXT("[SR] " Format), ##__VA_ARGS__)

#define LOG_SR_W(Format, ...) \
UE_LOG(LogTeam, Warning, TEXT("[SR][W] " Format), ##__VA_ARGS__)

#define LOG_SR_E(Format, ...) \
UE_LOG(LogTeam, Error, TEXT("[SR][E] " Format), ##__VA_ARGS__)


// =====================
// SM (팀원3)
// =====================
#define LOG_SM(Format, ...) \
UE_LOG(LogTeam, Log, TEXT("[SM] " Format), ##__VA_ARGS__)

#define LOG_SM_W(Format, ...) \
UE_LOG(LogTeam, Warning, TEXT("[SM][W] " Format), ##__VA_ARGS__)

#define LOG_SM_E(Format, ...) \
UE_LOG(LogTeam, Error, TEXT("[SM][E] " Format), ##__VA_ARGS__)