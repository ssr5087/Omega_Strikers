#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "OSType.generated.h"

// 공격이 맞았을 때 피격자에게 Interface로 전달할 공통 Hit Data
// 추후 상태이상에 대한 데이터가 추가적으로 들어올 수 있음
USTRUCT(BlueprintType)
struct FOSImpactData
{
	GENERATED_BODY()

	// 대상 넉백 방향
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D Direction = FVector2D::ZeroVector;

	// 코어 넉백 충격량
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CoreKnockbackPower = 0.f;

	// 플레이어 넉백 충격량
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlayerKnockbackPower = 0.f;

	// 플레이어 피격 피해량
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlayerDamage = 0.f;
};

// 팀 사이드 구분용
UENUM(BlueprintType)
enum class EOSTeam : uint8
{
	None,
	Core,
	Blue,
	Red
};

// 캐릭터의 현재 상태(사용 안 할 가능성 매우 높음)
UENUM(BlueprintType)
enum class EOSCharacterState : uint8
{
	Idle,
	Move,
	Skill,
	Hit,
	Disabled,
	Out
};

// 스킬 종류(역시 사용 안 할 가능성 매우 높음)
UENUM(BlueprintType)
enum class EOSSkillSlot : uint8
{
	Primary,
	Secondary,
	Ultimate
};