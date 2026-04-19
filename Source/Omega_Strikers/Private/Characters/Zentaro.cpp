// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/Zentaro.h"

AZentaro::AZentaro()
{
	PrimaryActorTick.bCanEverTick = true;

	// TODO: 데이터 셋 작업 되면 그걸로 사용
	// 기본 스탯
	MaxHP = 1000.f;
	Power = 55.f;
	Speed = 500.f;
	
	// 쿨다운
	
}

void AZentaro::BeginPlay()
{
	Super::BeginPlay();
}

void AZentaro::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AZentaro::PerformSlash(FVector Origin, FVector ForwardDir, float Renge, float HalfAngleDeg,
	const FOSImpactData& ImpactData)
{
}

void AZentaro::DoNormalStrike()
{
}

void AZentaro::DoChargeStrike()
{
}

void AZentaro::FirePenetratingSlash()
{
}

void AZentaro::Use_Flip()
{
	Super::Use_Flip();
}

void AZentaro::DoEnergyDodge()
{
}

void AZentaro::DoEnergyExplosion()
{
}

void AZentaro::EndDodge()
{
}

void AZentaro::Use_PrimarySkill()
{
	Super::Use_PrimarySkill();
}

void AZentaro::DoShatteredPhase1()
{
}

void AZentaro::DoShatteredPhase2()
{
}

void AZentaro::Use_SecondarySkill()
{
	Super::Use_SecondarySkill();
}

void AZentaro::ExecuteIawaseTeleport()
{
}

void AZentaro::Use_SpecialSkill()
{
	Super::Use_SpecialSkill();
}

void AZentaro::OniMultiHitTick()
{
}

void AZentaro::OniFinalBurst()
{
}

