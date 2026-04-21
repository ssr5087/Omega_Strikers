// Fill out your copyright notice in the Description page of Project Settings.


#include "Luna.h"


// Sets default values
ALuna::ALuna()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ALuna::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ALuna::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ALuna::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ALuna::Ready_CoreHit()
{
	Super::Ready_CoreHit();
}

void ALuna::Use_CoreHit()
{
	Super::Use_CoreHit();
}

void ALuna::Ready_PrimarySkill()
{
	
}

void ALuna::Use_PrimarySkill()
{
	// 스폰 - 걔가 알아서 이동 - 맨 처음 충돌 객체 저장 - 값 전달 - Destroy
	if (bPrimarySkillCoolDown) {return;}
	bPrimarySkillCoolDown = true;
	
	// 스폰하면서 그 친구에게 여러 가지 값들 전달
	
	
	// 쿨타임
	FTimerHandle PrimarySkillTimer;
	GetWorld()->GetTimerManager().SetTimer(
		PrimarySkillTimer,
		[this]()->void
		{
			bPrimarySkillCoolDown = false;
		},
		PrimarySkillCool,
		false
		);
}

void ALuna::Ready_SecondarySkill()
{
	
}

void ALuna::Use_SecondarySkill()
{
	// 직접 타고 이동 - 맨 처음 충돌 객체 저장 - 멈춤 - 저장된 객체에 값 전달
}

void ALuna::Ready_SpecialSkill()
{
	
}

void ALuna::Use_SpecialSkill()
{
	// 지정 위치에 z축 이동만 하는 로켓 소환
	// 바닥 좌표에 도달하는 순간 모든 적군과 코어에 방향과 충격량 각각 전달
}

void ALuna::Ready_Flip()
{
	
}

void ALuna::Use_Flip()
{
	
}
