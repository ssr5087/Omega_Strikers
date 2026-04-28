// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerBase.h"

#include "InputMappingContext.h"
#include "InputAction.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "OSTopDownController.h"
#include "Components/CapsuleComponent.h"
#include "Core/CoreBall.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Omega_Strikers/SM/HPComponent.h"
#include "Omega_Strikers/SM/OSPlayerController.h"
#include "Omega_Strikers/SSR/CharacterSkill.h"

// Sets default values
APlayerBase::APlayerBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// ========= Component =========
	
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	
	// 이동 관련 MovementComponent 조정
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;								// 플레이어가 이동 방향을 바라봄
	GetCharacterMovement()->RotationRate = FRotator(0.f, 50000.f, 0.f);	// 입력 즉시 바라보는 방향 바뀜
	GetCharacterMovement()->MaxAcceleration = 1000000.f;									// 방향 전환 시 미끄러지지 않고 즉시 전환
	GetCharacterMovement()->MaxWalkSpeed = 1000.f;	
	
	// HPComponent
	HPComp = CreateDefaultSubobject<UHPComponent>(TEXT("HPComp"));
	
	// =========== Input ===========
	ConstructorHelpers::FObjectFinder<UInputMappingContext> TempMap(TEXT("/Script/EnhancedInput.InputMappingContext'/Game/SM/Inputs/IMC_Player.IMC_Player'"));
	if (TempMap.Succeeded())
	{
		IMC_Player = TempMap.Object;
	}
	ConstructorHelpers::FObjectFinder<UInputAction> TempMove(TEXT("/Script/EnhancedInput.InputAction'/Game/SM/Inputs/IA_Move.IA_Move'"));
	if (TempMove.Succeeded())
	{
		IA_Move = TempMove.Object;
	}
	ConstructorHelpers::FObjectFinder<UInputAction> TempCore(TEXT("/Script/EnhancedInput.InputAction'/Game/SM/Inputs/IA_Core.IA_Core'"));
	if (TempCore.Succeeded())
	{
		IA_Core = TempCore.Object;
	}
	ConstructorHelpers::FObjectFinder<UInputAction> TempPrimary(TEXT("/Script/EnhancedInput.InputAction'/Game/SM/Inputs/IA_Primary.IA_Primary'"));
	if (TempPrimary.Succeeded())
	{
		IA_Primary = TempPrimary.Object;
	}
	ConstructorHelpers::FObjectFinder<UInputAction> TempSecondary(TEXT("/Script/EnhancedInput.InputAction'/Game/SM/Inputs/IA_Secondary.IA_Secondary'"));
	if (TempSecondary.Succeeded())
	{
		IA_Secondary = TempSecondary.Object;
	}
	ConstructorHelpers::FObjectFinder<UInputAction> TempSpecial(TEXT("/Script/EnhancedInput.InputAction'/Game/SM/Inputs/IA_Special.IA_Special'"));
	if (TempSpecial.Succeeded())
	{
		IA_Special = TempSpecial.Object;
	}
	ConstructorHelpers::FObjectFinder<UInputAction> TempFlip(TEXT("/Script/EnhancedInput.InputAction'/Game/SM/Inputs/IA_Flip.IA_Flip'"));
	if (TempFlip.Succeeded())
	{
		IA_Flip = TempFlip.Object;
	}
	
	// 데이터 셋
	CharacterName = "Default";
	Level = 1;
}

// Called when the game starts or when spawned
void APlayerBase::BeginPlay()
{
	Super::BeginPlay();
	
	// 스탯 초기 세팅중
	FCharacterStat* Stat = GetStatByLevel(Level);

	if (Stat)
	{
		ApplyStat(*Stat);
	}
	
	
	myPC = Cast<AOSTopDownController>(GetController());
	
	HPComp->InitializeHP();
	HPComp->OnHPBecomeNegative.BindUObject(this, &APlayerBase::KnockbackIncrease);
	HPComp->OnHPBecomePositive.BindUObject(this, &APlayerBase::KnockbackDecrease);
	
	// 임시 팀사이드
	TeamSide = EOSTeam::Blue;
}

// Called every frame
void APlayerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	
	// 마우스 커서 위치 기반 조준 방향 가져오기 (UI 및 공격에 사용)
	
	// 컨트롤러 캐스팅 실패 시
	if (!myPC) {myPC = Cast<AOSPlayerController>(GetController());}
	if (!myPC) {return;}
	
	// 마우스 위치 변환 성공 여부 저장
	bool bGetMousePointSuccess = myPC->GetMousePointOnArenaPlane(MouseCursorLoc);
	if (!bGetMousePointSuccess) {return;}
	
	// 벡터의 크기가 너무 작아 정규화가 실패한다면, 이전 프레임의 방향 벡터를 유지
	FVector TempCursorDir = MouseCursorLoc - GetActorLocation();
	FVector2D NewCursorDir = FVector2D(TempCursorDir.X, TempCursorDir.Y);
	if (!NewCursorDir.IsNearlyZero())
	{
		// 정규화가 충분히 가능하다면 갱신
		NewCursorDir.Normalize();
		CursorDir = NewCursorDir;
	}
	
	GEngine->AddOnScreenDebugMessage(2, DeltaTime, FColor::Cyan, FString::Printf(TEXT("조준 방향 : (%.2f, %.2f)"), CursorDir.X, CursorDir.Y));
}

// Called to bind functionality to input
void APlayerBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	// 입력 바인딩
	auto PC = Cast<APlayerController>(Controller);
	if (PC)
	{
		auto SubSystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
		if (SubSystem)
		{
			SubSystem->AddMappingContext(IMC_Player, 0);
		}
		
		auto playerInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
		if (playerInput)
		{
			playerInput->BindAction(IA_Move, ETriggerEvent::Triggered, this, &APlayerBase::PlayerMove);
			
			playerInput->BindAction(IA_Core, ETriggerEvent::Started, this, &APlayerBase::Ready_CoreHit);
			playerInput->BindAction(IA_Core, ETriggerEvent::Completed, this, &APlayerBase::Use_CoreHit);
			
			playerInput->BindAction(IA_Primary, ETriggerEvent::Started, this, &APlayerBase::Ready_PrimarySkill);
			playerInput->BindAction(IA_Primary, ETriggerEvent::Completed, this, &APlayerBase::Use_PrimarySkill);
			
			playerInput->BindAction(IA_Secondary, ETriggerEvent::Started, this, &APlayerBase::Ready_SecondarySkill);
			playerInput->BindAction(IA_Secondary, ETriggerEvent::Completed, this, &APlayerBase::Use_SecondarySkill);
			
			playerInput->BindAction(IA_Special, ETriggerEvent::Started, this, &APlayerBase::Ready_SpecialSkill);
			playerInput->BindAction(IA_Special, ETriggerEvent::Completed, this, &APlayerBase::Use_SpecialSkill);
			
			playerInput->BindAction(IA_Flip, ETriggerEvent::Started, this, &APlayerBase::Ready_Flip);
			playerInput->BindAction(IA_Flip, ETriggerEvent::Completed, this, &APlayerBase::Use_Flip);
		}
	}

}

void APlayerBase::PlayerMove(const FInputActionValue& InputActionValue)
{
	// 키보드 입력 값 정규화 및 속도 곱 전처리
	FVector2D value = InputActionValue.Get<FVector2D>();
	value.Normalize();
	
	// xy축 설정 기반 순간 이동량 추가
	FVector Forward = UKismetMathLibrary::GetForwardVector(FRotator(0.0f, GetControlRotation().Yaw, 0.0f));
	FVector Right = UKismetMathLibrary::GetRightVector(FRotator(0.0f, GetControlRotation().Yaw, GetControlRotation().Roll));
	AddMovementInput(Forward, value.X);
	AddMovementInput(Right, value.Y);
}

void APlayerBase::Ready_CoreHit()
{
	
}

void APlayerBase::Ready_PrimarySkill() {}

void APlayerBase::Ready_SecondarySkill() {}

void APlayerBase::Ready_SpecialSkill() {}

void APlayerBase::Ready_Flip() {}

void APlayerBase::Use_CoreHit()
{
	if (bCoreHitCoolDown) {return;}
	bCoreHitCoolDown = true;
	
	// 디버깅용 임시 코드
	// 임팩트 데이터
	FOSImpactData CoreImpactData;
	CoreImpactData.Direction = CursorDir;
	CoreImpactData.PlayerDamage = 0.f;
	CoreImpactData.PlayerKnockbackPower = 100.f;
	CoreImpactData.CoreKnockbackPower = 1230 + Power * 1.25f;
	
	// 코어 찾기
	AActor* Core = UGameplayStatics::GetActorOfClass(GetWorld(), ACoreBall::StaticClass());
	if (!Core) {return;}
	
	// 애니메이션 실행, 쿨타임 타이머 돌리기
	FTimerHandle CoreHitTimer;
	GetWorldTimerManager().SetTimer(
		CoreHitTimer,
		[this]()->void
		{
			bCoreHitCoolDown = false;
		},
		CoreHitCool,
		false
		);
	
	// 거리가 멀면 못 차요
	float CoreDist = FVector::Distance(Core->GetActorLocation(), GetActorLocation());
	if (CoreDist > 700.f) {return;}
	Execute_ReceiveImpact(Core, CoreImpactData, this);
}

void APlayerBase::Use_PrimarySkill() {}

void APlayerBase::Use_SecondarySkill() {}

void APlayerBase::Use_SpecialSkill() {}

void APlayerBase::Use_Flip() {}

bool APlayerBase::ReceiveImpact_Implementation(const FOSImpactData& ImpactData, AActor* InstigatorActor)
{
	// 1. 팀 사이드 체크 (공격자의 팀 사이드가 내 팀 사이드와 다를 경우에만 적용)
	if (ImpactData.TeamSide == TeamSide)
	{
		UE_LOG(LogTemp, Warning, TEXT("야 이놈아 맞았으면 말을 해"));
		return false;
	}
	
	// 2. 넉백부터 처리
	if (ImpactData.PlayerKnockbackPower > 0)
	{
		ApplyKnockback(ImpactData.Direction, ImpactData.PlayerKnockbackPower);
	}
	
	// 3. HPComp에서 데미지 적용
	if (HPComp && ImpactData.PlayerDamage > 0)
	{
		HPComp->ApplyDamage(ImpactData.PlayerDamage);
	}
	
	return true;
}

void APlayerBase::ApplyKnockback(FVector2D KnockbackDir, float KnockbackPow)
{
	// 2차원 벡터로 받아서 z성분이 0인 3차원 벡터로 변환(Launch 함수에 넣기 위함)
	FVector velocity = FVector(KnockbackDir.X, KnockbackDir.Y, 0.0f);
	LaunchCharacter(velocity * KnockbackPow * KnockbackRatio, true, false);
	
	// 밀려난 방향을 바라보게 설정
	SetActorRotation(UKismetMathLibrary::MakeRotFromXZ(-velocity,GetActorUpVector()));
}

void APlayerBase::KnockbackIncrease()
{
	// Stagger 상태에서는 넉백 거리가 3.5454배 증가, 간단하게 3.5로 맞춤
	KnockbackRatio = 3.5f;
}

void APlayerBase::KnockbackDecrease()
{
	// Stagger 원상복구. 추후 넉백 계수에 관한 장비 데이터가 존재할 경우 수정할 필요성 있음.
	KnockbackRatio = 1.f;
}

FCharacterStat* APlayerBase::GetStatByLevel(int32 InLevel)
{
	if (!StatTable) return nullptr;

	FString RowString = FString::Printf(TEXT("%s_%d"),
		*CharacterName.ToString(),
		InLevel);

	FName RowName = FName(*RowString);

	return StatTable->FindRow<FCharacterStat>(RowName, TEXT(""));
}

void APlayerBase::ApplyStat(const FCharacterStat& NewStat)
{
	CurrentStat = NewStat;

	// HP 갱신
	if (HPComp)
	{
		HPComp->UpdateMaxHP(NewStat.MaxHP);
	}
}

FCharacterSkill* APlayerBase::GetSkillData(FName SkillName)
{
	if (!SkillTable)
		return nullptr;
	
	return SkillTable->FindRow<FCharacterSkill>(SkillName, TEXT(""));
}

float APlayerBase::CalculateDamage(const FCharacterSkill& SkillData)
{
	float AttackPower = CurrentStat.Power;

	return SkillData.Damage_Flat +
		   (AttackPower * SkillData.Damage_Scale);
}

// 
FOSImpactData APlayerBase:: MakeImpactData(const FCharacterSkill& Skill)
{
	FOSImpactData Data;
	
	// 팀
	Data.TeamSide = TeamSide;
	// 방향
	Data.Direction = CursorDir;
	// 데미지
	Data.PlayerDamage = CalculateDamage(Skill);
	// 플레이어 넉백
	Data.PlayerKnockbackPower = Skill.PlayerKB_Flat + (CurrentStat.Power * Skill.PlayerKB_Scale);
	// 코어 넉백
	Data.CoreKnockbackPower = Skill.CoreKB_Flat + (CurrentStat.Power * Skill.CoreKB_Scale);
	
	return Data;
}