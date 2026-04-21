// Fill out your copyright notice in the Description page of Project Settings.


#include "Luna.h"

#include "Luna_PrimaryRocket.h"
#include "Components/CapsuleComponent.h"


// Sets default values
ALuna::ALuna()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	
	// ================= Component =================
	
	// 캡슐 컴포넌트 크기 조정
	GetCapsuleComponent()->SetCapsuleHalfHeight(165.0f);
	GetCapsuleComponent()->SetCapsuleRadius(102.0f);
	
	// 스켈레탈 메시 설정
	ConstructorHelpers::FObjectFinder<USkeletalMesh>TempSKM(TEXT("/Script/Engine.SkeletalMesh'/Game/Resource/Luna/Luna_Default_Lig.Luna_Default_Lig'"));
	if (TempSKM.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(TempSKM.Object);
	}
	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -162.0f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	GetMesh()->SetRelativeScale3D(FVector(3.0f));
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	// 주 스킬 사용 시 발사할 로켓 위치
	RocketLauncher = CreateDefaultSubobject<USceneComponent>(TEXT("RocketLauncher"));
	RocketLauncher->SetupAttachment(RootComponent);
	RocketLauncher->SetRelativeLocation(FVector(270.0f, 0.0f, 0.0f));
	
	
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
	// [Primary] 스폰 - 걔가 알아서 이동 - 맨 처음 충돌 객체 저장 - 값 전달 - Destroy
	
	// 현재 쿨타임 중이면 실행 안 됨
	if (bPrimarySkillCoolDown) {return;}

	if (!RocketFactory)
	{
		UE_LOG(LogTemp, Error, TEXT("RocketFactory is null"));
		return;
	}

	if (!GetWorld())
	{
		UE_LOG(LogTemp, Error, TEXT("World is null"));
		return;
	}
	
	// 스폰하면서 그 친구에게 여러 가지 값들 전달
	FTransform LauncherTransform = RocketLauncher->GetComponentTransform();
	GetWorld()->SpawnActor<ALuna_PrimaryRocket>(RocketFactory, LauncherTransform);
	
	// 쿨타임
	bPrimarySkillCoolDown = true;
	FTimerHandle PrimarySkillTimer;
	GetWorld()->GetTimerManager().SetTimer(PrimarySkillTimer, [this]()->void {bPrimarySkillCoolDown = false;}, PrimarySkillCool, false);
}

void ALuna::Ready_SecondarySkill()
{
	
}

void ALuna::Use_SecondarySkill()
{
	// [Secondary] 직접 타고 이동 - 맨 처음 충돌 객체 저장 - 멈춤 - 저장된 객체에 값 전달
	if (bSecondarySkillCoolDown) {return;}
	
	
	// 쿨타임
	bSecondarySkillCoolDown = true;
	FTimerHandle SecondarySkillTimer;
	GetWorld()->GetTimerManager().SetTimer(SecondarySkillTimer, [this]()->void {bSecondarySkillCoolDown = false;}, SecondarySkillCool, false);
}

void ALuna::Ready_SpecialSkill()
{
	
}

void ALuna::Use_SpecialSkill()
{
	// [Special] 지정 위치에 z축 이동만 하는 로켓 소환
	// 바닥 좌표에 도달하는 순간 모든 적군과 코어에 방향과 충격량 각각 전달
	if (bSpecialSkillCoolDown) {return;}
	
	
	// 쿨타임
	bSpecialSkillCoolDown = true;
	FTimerHandle SpecialSkillTimer;
	GetWorld()->GetTimerManager().SetTimer(SpecialSkillTimer, [this]()->void {bSpecialSkillCoolDown = false;}, SpecialSkillCool, false);
}

void ALuna::Ready_Flip()
{
	
}

void ALuna::Use_Flip()
{
	
}
