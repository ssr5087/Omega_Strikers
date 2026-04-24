// Fill out your copyright notice in the Description page of Project Settings.


#include "Luna.h"

#include "Luna_PrimaryRocket.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"


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
	
	TeamSide = EOSTeam::Red;
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
	
	// 스폰 트랜스폼 만들기
	FTransform LauncherTransform;
	
	// 발사 방향 (플레이어 -> 커서 방향)
	FVector PlayerLoc = GetActorLocation();
	float zLoc = PlayerLoc.Z - 50.f;
	PlayerLoc = FVector(PlayerLoc.X, PlayerLoc.Y, zLoc);
	FVector LaunchDir = FVector(CursorDir.X, CursorDir.Y, 0);
	FRotator SpawnRot = UKismetMathLibrary::MakeRotFromXZ(LaunchDir, GetActorUpVector());
	
	LauncherTransform.SetLocation(PlayerLoc + LaunchDir * 270);
	LauncherTransform.SetRotation(SpawnRot.Quaternion());
	
	// 일단 스폰 파라미터에 누가 스폰했는지만 넣기
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	// SpawnParams.Instigator = GetInstigator();
	// SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	
	ALuna_PrimaryRocket* Rocket = GetWorld()->SpawnActor<ALuna_PrimaryRocket>(RocketFactory, LauncherTransform, SpawnParams);

	// FOSImpactData PrimaryImpactData;
	// PrimaryImpactData.Direction;
	// PrimaryImpactData.CoreKnockbackPower;
	// PrimaryImpactData.PlayerKnockbackPower;
	// PrimaryImpactData.PlayerDamage;
	if (Rocket)
	{
		Rocket->InitRocket(Power, this, TeamSide);
	}
	
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
