// Fill out your copyright notice in the Description page of Project Settings.


#include "Luna.h"

#include "HPComponent.h"
#include "InputActionValue.h"
#include "Luna_PrimaryRocket.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Omega_Strikers/Omega_Strikers.h"


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
	ConstructorHelpers::FObjectFinder<USkeletalMesh>TempSKM(TEXT("/Script/Engine.SkeletalMesh'/Game/Resource/Luna/Animations/SK_ChaoticRocketeer_Default/SkeletalMeshes/SK_ChaoticRocketeer_Default.SK_ChaoticRocketeer_Default'"));
	if (TempSKM.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(TempSKM.Object);
	}
	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -162.0f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	GetMesh()->SetRelativeScale3D(FVector(3.0f));
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	// // 주 스킬 사용 시 발사할 로켓 위치
	// RocketLauncher = CreateDefaultSubobject<USceneComponent>(TEXT("RocketLauncher"));
	// RocketLauncher->SetupAttachment(RootComponent);
	// RocketLauncher->SetRelativeLocation(FVector(270.0f, 0.0f, 0.0f));
	
	// 보조 스킬 히트 판정에 사용할 충돌 컴포넌트
	SecondaryHitBox = CreateDefaultSubobject<UBoxComponent>("SecondaryHitBox");
	SecondaryHitBox->SetupAttachment(RootComponent);
	SecondaryHitBox->SetBoxExtent(FVector(150.0f, 100.0f, 150.0f));
	SecondaryHitBox->OnComponentBeginOverlap.AddDynamic(this, &ALuna::OnDashOverlap);
}

// Called when the game starts or when spawned
void ALuna::BeginPlay()
{
	Super::BeginPlay();
	
	TeamSide = EOSTeam::Red;
	
	// 데이터 셋 세팅
	FCharacterStat* Stat = GetStatByLevel(Level);
	
	if (Stat)
	{
		ApplyStat(*Stat);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Stat Load Failed"));
	}
	
	UE_LOG(LogTemp, Warning, TEXT("CharacterName: %s"), *CharacterName.ToString());
}

// Called every frame
void ALuna::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	GEngine->AddOnScreenDebugMessage(300000, 1, FColor::Magenta, FString::Printf(TEXT("%.2f, %.2f"), CurDir.X, CurDir.Y));
}

// Called to bind functionality to input
void ALuna::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ALuna::PlayerMove(const struct FInputActionValue& InputActionValue)
{
	if (bIsProcessingSecondary)
	{
		// 키보드 입력 값 정규화 및 속도 곱 전처리
		
		FVector2D AimDir = InputActionValue.Get<FVector2D>();
		NewDir = FVector(AimDir.X, AimDir.Y, 0.0f);
		//GEngine->AddOnScreenDebugMessage(300000, 1, FColor::Magenta, FString::Printf(TEXT("%.2f, %.2f"), value.X, value.Y));
		
		// 방향 전환 입력했으니 반영되도록 설정
		bIsChangingDirection = true;
		
		return;
	}
	Super::PlayerMove(InputActionValue);
}

FCharacterStat* ALuna::GetStatByLevel(int32 InLevel)
{
	if (!CharacterStatTable)
	{
		UE_LOG(LogTemp, Error, TEXT("CharacterStatTable is NULL"));
		return nullptr;
	}
	
	FName RowName = FName(*FString::Printf(TEXT("%s_%d"), *CharacterName.ToString(), InLevel));
	
	UE_LOG(LogTemp, Warning, TEXT("Trying Row: %s"), *RowName.ToString());
	
	return CharacterStatTable->FindRow<FCharacterStat>(RowName, TEXT(""));
}

void ALuna::ApplyStat(const FCharacterStat& Stat)
{
	UE_LOG(LogTemp, Warning, TEXT("ApplyStat 실행됨"));
	CurrentStat = Stat;
	
	// PlayerBase 변수 덮어쓰기
	MaxHP = Stat.MaxHP;
	Power = Stat.Power;
	Speed = Stat.Speed;
	CoolDownRate = Stat.Cooldown;
	
	// 이동속도 적용
	GetCharacterMovement()->MaxWalkSpeed = Speed;
	
	if (HPComp)
	{
		HPComp->UpdateMaxHP(MaxHP);
		HPComp->InitializeHP();
	}
	
	// 테스트 용
	UE_LOG(LogTemp, Warning, TEXT("HP: %.1f / Power: %.1f / Speed: %.1f"),
	MaxHP, Power, Speed);
}

void ALuna::LevelUp()
{
	Level++;
	
	FCharacterStat* Stat = GetStatByLevel(Level);
	
	if (Stat)
	{
		ApplyStat(*Stat);
	}
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
	float zLoc = 50.f;
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
	
	ALuna_PrimaryRocket* Rocket = GetWorld()->SpawnActorDeferred<ALuna_PrimaryRocket>(RocketFactory, LauncherTransform);

	// FOSImpactData PrimaryImpactData;
	// PrimaryImpactData.Direction;
	// PrimaryImpactData.CoreKnockbackPower;
	// PrimaryImpactData.PlayerKnockbackPower;
	// PrimaryImpactData.PlayerDamage;
	if (Rocket)
	{
		Rocket->InitRocket(Power, this, TeamSide);
		Rocket->FinishSpawning(LauncherTransform);
	}
	
	// 쿨타임 관리
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
	bSecondarySkillCoolDown = true;
	bIsProcessingSecondary = true;
	GetCharacterMovement()->MaxWalkSpeed = 10000.f;
	
	// 최초 이동 방향
	CurDir = FVector(CursorDir.X, CursorDir.Y, 0.0f);
	
	// 업데이트 함수 타이머 호출
	GetWorldTimerManager().SetTimer(MoveTimer, this, &ALuna::Update_SecondaryMove, 0.1f, true);
	
	// 쿨타임 관리
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
	
	
	// 쿨타임 관리
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

void ALuna::Update_SecondaryMove()
{
	// xy축 설정 기반 순간 이동량 추가
	FVector Forward = UKismetMathLibrary::GetForwardVector(FRotator(0.0f, GetControlRotation().Yaw, 0.0f));
	FVector Right = UKismetMathLibrary::GetRightVector(FRotator(0.0f, GetControlRotation().Yaw, GetControlRotation().Roll));
	AddMovementInput(Forward, CurDir.X);
	AddMovementInput(Right, CurDir.Y);
	
	if (!bIsChangingDirection) {return;}
	CurDir = FMath::VInterpTo(CurDir, NewDir, 0.1, 1);
	CurDir.Normalize();
	
	// 방향 입력 종료 시 방향 전환 종료
	bIsChangingDirection = false;
}

void ALuna::OnDashOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 충격 전달 구조체
	FOSImpactData SecondaryImpactData;
	SecondaryImpactData.TeamSide = TeamSide;
	SecondaryImpactData.Direction = FVector2D(CurDir.X, CurDir.Y);
	SecondaryImpactData.CoreKnockbackPower = 3000.f;
	SecondaryImpactData.PlayerKnockbackPower = 1000.f;
	SecondaryImpactData.PlayerDamage = 200.f;
	
	// 초기화 분기용 변수
	bool bIsSuccess = false;
	
	// 자기 자신 충돌 무시
	if (OtherActor == this) {return;}
	
	// OtherActor가 존재하는 경우에만 실행
	if (OtherActor)
	{
		// 벽과 충돌했다면 초기화
		if (OtherActor->GetName().Contains("Arena"))
		{
			bIsSuccess = true;
		}
		
		// 앞에서 초기화 활성화 되었으면 실행 x
		// 충돌 대상이 인터페이스 함수 구현했다면
		if (!bIsSuccess && OtherActor->Implements<UOSImpactReceiver>())
		{
			// 그 대상에 대해 인터페이스 함수를 실행해라
			bIsSuccess = IOSImpactReceiver::Execute_ReceiveImpact(OtherActor, SecondaryImpactData, this);
		}
		
		// 성공하면 초기화
		if (bIsSuccess)
		{
			LOG_SM_W(TEXT("성공했냐?"));
			GetWorldTimerManager().ClearTimer(MoveTimer);
			GetCharacterMovement()->MaxWalkSpeed = 1000.f;
			bIsProcessingSecondary = false;
		}
	}
}

