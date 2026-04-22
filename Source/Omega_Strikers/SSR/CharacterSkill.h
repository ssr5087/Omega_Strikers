#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "CharacterSkill.generated.h"

UENUM(BlueprintType)
enum class ESkillType : uint8
{
	Strike,
	Evade,
	EnergyBurst,
	Primary,
	Secondary,
	Special
};

USTRUCT(BlueprintType)
struct FCharacterSkill : public FTableRowBase
{
	GENERATED_BODY()
public:
	
	// 스킬 타입 (Primary, Secondary 등)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESkillType SkillType;

	// 쿨타임
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CoolTime = 0.f;

	// Core Knockback
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CoreKB_Flat = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CoreKB_Scale = 0.f;

	// Player Knockback
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlayerKB_Flat = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlayerKB_Scale = 0.f;

	// Damage
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damage_Flat = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damage_Scale = 0.f;
};
