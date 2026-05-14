// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerBase.h"

#include "InputMappingContext.h"
#include "InputAction.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Core/CoreBall.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Omega_Strikers/Omega_Strikers.h"
#include "Omega_Strikers/SM/HPComponent.h"
#include "Omega_Strikers/SM/HPStatusWidget.h"
#include "Omega_Strikers/SM/OSPlayerController.h"
#include "Omega_Strikers/SSR/CharacterSkill.h"
#include "Omega_Strikers/SSR/EXPComponent.h"
#include "SkillIndicatorBase.h"

// Sets default values
APlayerBase::APlayerBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 동기화 및 움직임 동기화 설정
	bReplicates = true;
	SetReplicateMovement(true);
	
	// ========= Component =========
	
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_EXPOrb, ECR_Overlap);
	
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
	
	// EXP 생성자
	EXPComp = CreateDefaultSubobject<UEXPComponent>("EXPComp");
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
	
	if (IsLocallyControlled())
	{
		myPC = Cast<AOSPlayerController>(GetController());
	}
	
	HPComp->InitializeHP();
	HPComp->OnHPBecomeNegative.BindUObject(this, &APlayerBase::KnockbackIncrease);
	HPComp->OnHPBecomePositive.BindUObject(this, &APlayerBase::KnockbackDecrease);
	
	if (EXPComp)
	{
		EXPComp->OnLevelUp.AddUniqueDynamic(this, &APlayerBase::HandleLevelUp);
	}
}

// ════════════════════════════════════════════════════════════
//  ClearAllAiming
// ════════════════════════════════════════════════════════════
void APlayerBase::ClearAllAiming()
{
	bAimingCoreHit   = false;
	bAimingPrimary   = false;
	bAimingSecondary = false;
	bAimingSpecial   = false;
	HideSkillIndicator();
}

void APlayerBase::ShowSkillIndicator(TSubclassOf<ASkillIndicatorBase> IndicatorClass, ESkillType SkillType)
{
	if (!IsLocallyControlled())
	{
		return;
	}

	HideSkillIndicator();

	UClass* SpawnClass = IndicatorClass ? IndicatorClass.Get() : ASkillIndicatorBase::StaticClass();
	if (!SpawnClass || !GetWorld())
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	CurrentIndicator = GetWorld()->SpawnActor<ASkillIndicatorBase>(
		SpawnClass,
		GetActorLocation(),
		GetActorRotation(),
		SpawnParams);

	if (!CurrentIndicator)
	{
		return;
	}

	ConfigureSkillIndicator(SkillType, CurrentIndicator);
	CurrentIndicator->UpdateIndicator(this);
}

void APlayerBase::HideSkillIndicator()
{
	if (!CurrentIndicator)
	{
		return;
	}

	CurrentIndicator->Destroy();
	CurrentIndicator = nullptr;
}

void APlayerBase::ConfigureSkillIndicator(ESkillType SkillType, ASkillIndicatorBase* Indicator)
{
	(void)SkillType;
	(void)Indicator;
}

// Called every frame
void APlayerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// 각 로컬에서 조종 중인 캐릭터에게만 커서 값 가져오기
	// 서버도 자신이 조종하는 캐릭터의 마우스는 갖고 와야 함
	if (IsLocallyControlled())
	{
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
		
		//GEngine->AddOnScreenDebugMessage(2, DeltaTime, FColor::Cyan, FString::Printf(TEXT("조준 방향 : (%.2f, %.2f)"), CursorDir.X, CursorDir.Y));
	}

	if (IsLocallyControlled() && CurrentIndicator)
	{
		CurrentIndicator->UpdateIndicator(this);
	}
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
	// 클라에서만 실행
	if (!IsLocallyControlled())
	{
		return;
	}
	
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
	ClearAllAiming();
	bAimingCoreHit = true;
}

void APlayerBase::Ready_PrimarySkill()
{
	ClearAllAiming();
	bAimingPrimary = true;
}

void APlayerBase::Ready_SecondarySkill()
{
	ClearAllAiming();
	bAimingSecondary = true;
}

void APlayerBase::Ready_SpecialSkill()
{
	ClearAllAiming();
	bAimingSpecial = true;
}

void APlayerBase::Ready_Flip() {}

void APlayerBase::Use_CoreHit()
{
	ClearAllAiming();
	
	// 클라에서만 실행
	if (!IsLocallyControlled()) {return;}
	
	ServerRPC_CoreHit(CursorDir);
}

void APlayerBase::Use_PrimarySkill() { ClearAllAiming(); }

void APlayerBase::Use_SecondarySkill() { ClearAllAiming(); }

void APlayerBase::Use_SpecialSkill() { ClearAllAiming(); }

void APlayerBase::Use_Flip() {}

void APlayerBase::ApllyTeamSide(EOSTeam team)
{
	if (HasAuthority())
	{
		// 서버에서만 팀 지정, 클라에서 동기화
		TeamSide = team;
	}
}

bool APlayerBase::ReceiveImpact_Implementation(const FOSImpactData& ImpactData, AActor* InstigatorActor)
{
	// 넉백, 체력 상태는 서버에서만 관리
	if (!HasAuthority()) {return false;}
	if (bIsProcessingImpact)
	{
		return false;
	}
	TGuardValue<bool> ImpactGuard(bIsProcessingImpact, true);
	
	LOG_SR_W(TEXT("Dir : %s"), *ImpactData.Direction.ToString());
	
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
	// 넉백, 체력 상태는 서버에서만 관리
	if (!HasAuthority()) {return;}
	
	// SSR 테스트 용
	
	// 2차원 벡터로 받아서 z성분이 0인 3차원 벡터로 변환(Launch 함수에 넣기 위함)
	FVector velocity = FVector(KnockbackDir.X, KnockbackDir.Y, 0.0f).GetSafeNormal();
	LaunchCharacter(velocity * KnockbackPow * KnockbackRatio, true, true);
	
	// 밀려난 방향을 바라보게 설정
	SetActorRotation(UKismetMathLibrary::MakeRotFromXZ(-velocity,GetActorUpVector()));
}

void APlayerBase::KnockbackIncrease()
{
	// 서버에서만 실행
	if (!HasAuthority()) {return;}
	
	// Stagger 상태에서는 넉백 거리가 3.5454배 증가, 간단하게 3.5로 맞춤
	KnockbackRatio = 17.5f;
}

void APlayerBase::KnockbackDecrease()
{
	// 서버에서만 실행
	if (!HasAuthority()) {return;}
	
	// Stagger 원상복구. 추후 넉백 계수에 관한 장비 데이터가 존재할 경우 수정할 필요성 있음.
	KnockbackRatio = 5.f;
}

void APlayerBase::OnRep_Level()
{
	HandleLevelUp(Level);
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
	Data.CoreKnockbackPower *= 3.0f;
	
	LOG_SR_W(TEXT("데미지 들어감!!"));
	
	return Data;
}

void APlayerBase::ServerRPC_CoreHit_Implementation(FVector2D HitDir)
{
	// 서버에서 쿨타임 체크, 사용 가능할 때만 사용
	if (bCoreHitCoolDown) {return;}
	
	// 데이터 셋 불러오기
	FCharacterSkill* Skill = GetSkillData(FName(TEXT("Aimi_Strike")));
	
	// 데이터 셋 로드 실패 시 리턴
	if (!Skill) {return;}
			
	// ImpactData를 데이터 셋으로부터 계산하여 설정
	// 단, 방향 값은 서버의 마우스 커서가 아닌 클라이언트의 마우스 커서로 계산
	FOSImpactData CoreImpactData;
	CoreImpactData = MakeImpactData(*Skill);
	CoreImpactData.Direction = HitDir;
	
	// 코어 찾기
	if ( !CachedCoreBall )
	{
		AActor* core = UGameplayStatics::GetActorOfClass(GetWorld(), ACoreBall::StaticClass());
		CachedCoreBall = Cast<ACoreBall>(core);

		// 그래도 없으면 리턴
		if ( !CachedCoreBall )
		{
			LOG_GT_E(TEXT("CoreBall을 찾지 못했다!")) return;
		}
	}
	
	// 애니메이션 실행, 쿨타임 타이머 돌리기
	bCoreHitCoolDown = true;
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
	float CoreDist = FVector::Distance(CachedCoreBall->GetActorLocation(), GetActorLocation());
	if (CoreDist > 700.f) {return;}

	Execute_ReceiveImpact(CachedCoreBall, CoreImpactData, this);
	// // ✅ CoreBall은 Server RPC로 처리 -> 이미 값 전달 후 물리는 서버에서만 적용되도록 CoreBall 내부 로직이 짜여 있음
	// if ( CachedCoreBall )
	// {
	// 	FVector KnockDir = FVector(HitDir.X, HitDir.Y, 0.f).GetSafeNormal();
	// 	CachedCoreBall->Server_HitCore(GetActorLocation(), KnockDir, CoreImpactData.CoreKnockbackPower);
	// }
	// else
	// {
	// 	Execute_ReceiveImpact(CachedCoreBall, CoreImpactData, this);
	// }
}

void APlayerBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(APlayerBase, TeamSide);
	DOREPLIFETIME(APlayerBase, KnockbackRatio);
	
	// EXP
	DOREPLIFETIME(APlayerBase, Level);
}

// EXPComp 레벨업 관리
void APlayerBase::HandleLevelUp(int32 NewLevel)
{
	LOG_SR_W(TEXT("Player LevelIp : %d"), NewLevel);
	
	FCharacterStat* Stat = GetStatByLevel(NewLevel);
	
	if (Stat)
	{
		ApplyStat(*Stat);
	}
	
	// 여기서 이펙트
	// PlayLevelupEffect();
}
