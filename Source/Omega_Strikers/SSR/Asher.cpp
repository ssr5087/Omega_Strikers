// Fill out your copyright notice in the Description page of Project Settings.


#include "Asher.h"

#include "Asher_Special_Projectile.h"
#include "Asher_Special_Shield.h"
#include "Core/CoreBall.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Asher_AnimInstance.h"
#include "Net/UnrealNetwork.h"
#include "SkillIndicatorBase.h"
#include "Omega_Strikers/Omega_Strikers.h"
#include "Omega_Strikers/SM/LunaSkillCool.h"


// Sets default values
AAsher::AAsher()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SetNetUpdateFrequency(100.f);
	SetMinNetUpdateFrequency(60.f);
	
	
	// 스켈레탈 메시 설정
	ConstructorHelpers::FObjectFinder<USkeletalMesh>TempSKM(TEXT("/Script/Engine.SkeletalMesh'/Game/Resource/Asher/Animations/SK_ShieldUser_Default/SkeletalMeshes/SK_ShieldUser_Default.SK_ShieldUser_Default'"));
	if (TempSKM.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(TempSKM.Object);
	}
	GetMesh()->SetRelativeLocation(FVector(0.000000,0.000000,-90.000000));
	GetMesh()->SetRelativeRotation(FRotator(0.000000,-89.999999,0.000000));
	GetMesh()->SetRelativeScale3D(FVector(3.0f));
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 데이터 셋
	CharacterName = "Asher";
}

// Called when the game starts or when spawned
void AAsher::BeginPlay()
{
	Super::BeginPlay();
	
	// 내가 조작 중인 캐릭터가 애셔일 때만 UI 붙이기
	if (IsLocallyControlled())
	{
		// UI 붙이기
		SkillUI = CreateWidget<ULunaSkillCool>(GetWorld(), CoolTimeUI);
		if (SkillUI)
		{
			SkillUI->AddToViewport();
		}
	}
	
	
	UE_LOG(LogTemp, Warning, TEXT("Power: %.1f"), CurrentStat.Power);
}

// 스킬 사거리 표시
void AAsher::ConfigureSkillIndicator(ESkillType SkillType, ASkillIndicatorBase* Indicator)
{
	Super::ConfigureSkillIndicator(SkillType, Indicator);

	if (!Indicator)
	{
		return;
	}

	float IndicatorRange = 0.f;

	switch (SkillType)
	{
	case ESkillType::Primary:
		IndicatorRange = 175.f;
		break;

	case ESkillType::Secondary:
		IndicatorRange = Secondary_DashDistance;
		break;

	case ESkillType::Special:
		IndicatorRange = 500.f;
		break;

	default:
		return;
	}

	Indicator->SetIndicatorRange(IndicatorRange);
}

// Called every frame
void AAsher::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AAsher::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AAsher::Ready_CoreHit()
{
	Super::Ready_CoreHit();
	// UI 쪽
}

void AAsher::Ready_PrimarySkill()
{
	// 쿨타임 중일때는 사용금지
	if (bPrimary_SkillCoolDown)
		return;
	// 콤보중일때도 사용 금지
	if (bIsPrimary_Attacking)
		return;

	Super::Ready_PrimarySkill();
	ShowSkillIndicator(PrimaryIndicatorClass, ESkillType::Primary);
}

void AAsher::Ready_SecondarySkill()
{
	if (bSecondary_SkillCoolDown || bIsSecondary_Dashing)
	{
		return;
	}

	Super::Ready_SecondarySkill();
	
	ShowSkillIndicator(SecondaryIndicatorClass, ESkillType::Secondary);
}

void AAsher::Ready_SpecialSkill()
{
	// 쿨타임 중일때는 사용금지
	if (bSpecial_SkillCoolDown)
		return;

	Super::Ready_SpecialSkill();
	ShowSkillIndicator(SpecialIndicatorClass, ESkillType::Special);
}

void AAsher::Ready_Flip()
{
	Super::Ready_Flip();
}

void AAsher::Use_CoreHit()
{
	bAimingCoreHit = false;

	if (!IsLocallyControlled())
		return;
	
	if (SkillUI)
		SkillUI->LoadCore();
	

	ServerRPC_StartCoreHit(CursorDir);
	ServerRPC_CoreHit(CursorDir);
}

void AAsher::Use_PrimarySkill()
{
	Super::Use_PrimarySkill();
	
	if (!IsLocallyControlled())
	{
		return;
	}
	

	ServerRPC_StartPrimarySkill(CursorDir);
}

void AAsher::Use_SecondarySkill()
{
	Super::Use_SecondarySkill();

	if (!IsLocallyControlled())
	{
		return;
	}

	ServerRPC_StartSecondarySkill(CursorDir);
}

void AAsher::Use_SpecialSkill()
{
	Super::Use_SpecialSkill();
	
	if (!IsLocallyControlled())
		return;

	ServerRPC_StartSpecialSkill(CursorDir);
}

void AAsher::Use_Flip()
{
	Super::Use_Flip();
}

void AAsher::DoPrimaryHit1()
{
	FVector Forward = FVector(CursorDir.X, CursorDir.Y, 0.f).GetSafeNormal();
	FVector Center = GetActorLocation() + Forward * 150.f;

	TArray<FHitResult> Hits;

	UKismetSystemLibrary::SphereTraceMulti(
		GetWorld(),
		Center,
		Center,
		250.f, // 기본값 150
		UEngineTypes::ConvertToTraceType(ECC_Pawn),
		false,
		TArray<AActor*>(),
		EDrawDebugTrace::None,
		Hits,
		true
	);

	for (auto& Hit : Hits)
	{
		AActor* Target = Hit.GetActor();
		if (!Target || Target == this) continue;

		if (!Target->Implements<UOSImpactReceiver>())
		{
			UE_LOG(LogTemp, Warning, TEXT("❌ Not ImpactReceiver: %s"), *Target->GetName());
			continue;
		}
		
		if (HitActors.Contains(Target)) continue;

		// FOSImpactData Data;
		// Data.Direction = CursorDir;
		// Data.PlayerDamage = 100.f;
		// Data.CoreKnockbackPower = 1000.f;
		// Data.PlayerKnockbackPower = 600.f;
		
		FCharacterSkill* Skill = GetSkillData(TEXT("Asher_Primary_Projectile"));
		if (!Skill)
			return;
		
		FOSImpactData Data = MakeImpactData(*Skill);

		IOSImpactReceiver::Execute_ReceiveImpact(Target, Data, this);

		HitActors.Add(Target);
	}
	HitActors.Empty();
}

void AAsher::DoPrimaryHit2()
{
	FVector Forward = FVector(CursorDir.X, CursorDir.Y, 0.f).GetSafeNormal();
	FVector Center = GetActorLocation() + Forward * 200.f;

	TArray<FHitResult> Hits;

	UKismetSystemLibrary::SphereTraceMulti(
		GetWorld(),
		Center,
		Center,
		400.f,
		UEngineTypes::ConvertToTraceType(ECC_Pawn),
		false,
		TArray<AActor*>(),
		EDrawDebugTrace::None,
		Hits,
		true
	);

	for (auto& Hit : Hits)
	{
		AActor* Target = Hit.GetActor();
		if (!Target || Target == this) continue;
		
		if (!Target->Implements<UOSImpactReceiver>())
		{
			UE_LOG(LogTemp, Warning, TEXT("❌ Not ImpactReceiver: %s"), *Target->GetName());
			continue;
		}
		
		UE_LOG(LogTemp, Warning, TEXT("첫 번째 컨티뉴 통과"));
		
		if (HitActors.Contains(Target))
			continue;
		HitActors.Add(Target);
		UE_LOG(LogTemp, Warning, TEXT("두 번째 컨티뉴 통과"));
		// 👉 1타 맞은 애는 또 맞을 수 있음 (OK)
		// 대신 2타 중복만 막으면 됨

		FVector ToTarget = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();

		float Dot = FVector::DotProduct(Forward, ToTarget);
		
		FCharacterSkill* Skill = GetSkillData(TEXT("Asher_Primary_Projectile"));
		if (!Skill)
			return;
		
		

		if (Dot > 0.8f) //  중앙 판정
		{
			
			Skill = GetSkillData("Asher_Primary_Center");
		}
		else if (Dot > 0.3f)    // 사이드
		{
			
			Skill = GetSkillData("Asher_Primary_Side");
		}
		else
		{
			continue; // 뒤쪽 무시
		}
		
		if (!Skill)
			continue;
		
		FOSImpactData Data = MakeImpactData(*Skill);

		

		IOSImpactReceiver::Execute_ReceiveImpact(Target, Data, this);
		
	}
	HitActors.Empty();
	
	// 쿨타임
	bPrimary_SkillCoolDown = true;
	FTimerHandle PrimarySkillTimer;
	GetWorld()->GetTimerManager().SetTimer(PrimarySkillTimer, [this]()->void {bPrimary_SkillCoolDown = false;}, Primary_SkillCool, false);
}

void AAsher::DoSpecialProjectile()
{
	FVector Forward = FVector(CursorDir.X, CursorDir.Y, 0.f).GetSafeNormal();
	if (Forward.IsNearlyZero())
	{
		Forward = GetActorForwardVector();
		Forward.Z = 0.f;
		Forward.Normalize();
	}

	FVector SpawnLocation = GetActorLocation() + Forward * 200.f;

	auto Projectile = GetWorld()->SpawnActor<AAsher_Special_Projectile>(
		SpecialProjectileClass,
		SpawnLocation,
		Forward.Rotation()
	);

	if (Projectile)
	{
		Projectile->SetOwner(this);

		// 🔥 방향 + 팀 전달
		Projectile->Init(Forward, MyTeam); // MyTeam은 네 캐릭터 팀 변수

		// 🔥 히트 시 방패 생성 연결
		Projectile->OnHit.BindUObject(this, &AAsher::DoSpecialShield);
	}
}

void AAsher::DoSpecialShield(FVector SpawnLocation, FVector Direction)
{
	auto Shield = GetWorld()->SpawnActor<AAsher_Special_Shield>(
		SpecialShieldClass,
		SpawnLocation,
		FRotator::ZeroRotator
	);

	if (Shield)
	{
		Shield->SetOwner(this);

		// 기존 경로를 유지하려면 방향 기반 회전값도 같이 전달해야 함
		Shield->Init(Direction, Direction.Rotation());
	}
}

void AAsher::DoSecondaryDash()
{
	if (!HasAuthority())
	{
		return;
	}

	if (!GetWorld())
	{
		return;
	}

	bIsSecondary_Dashing = true;
	SecondaryLastTraceLocation = GetActorLocation();

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->StopMovementImmediately();
		LaunchCharacter(SecondaryDashDirection * (Secondary_DashDistance / Secondary_DashDuration), true, false);
	}

	GetWorldTimerManager().SetTimer(
		SecondaryDashTimer,
		this,
		&AAsher::DoSecondaryDashTrace,
		Secondary_DashTraceInterval,
		true
	);

	FTimerHandle SecondaryDashEndTimer;
	GetWorldTimerManager().SetTimer(
		SecondaryDashEndTimer,
		this,
		&AAsher::EndSecondaryDash,
		Secondary_DashDuration,
		false
	);

	GetWorldTimerManager().SetTimer(
		SecondarySkillTimer,
		[this]()
		{
			bSecondary_SkillCoolDown = false;
		},
		Secondary_SkillCool,
		false
	);
}

void AAsher::DoSecondaryDashTrace()
{
	if (!HasAuthority())
	{
		return;
	}

	if (!GetWorld())
	{
		return;
	}

	const FVector CurrentLocation = GetActorLocation();
	const FVector TraceStart = SecondaryLastTraceLocation;
	const FVector TraceEnd = CurrentLocation;

	if (TraceStart.Equals(TraceEnd, 1.f))
	{
		return;
	}

	TArray<FHitResult> Hits;
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	GetWorld()->SweepMultiByObjectType(
		Hits,
		TraceStart,
		TraceEnd,
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(Secondary_HitRadius),
		QueryParams
	);

	for (const FHitResult& Hit : Hits)
	{
		AActor* Target = Hit.GetActor();
		if (!Target || SecondaryHitActors.Contains(Target))
		{
			continue;
		}

		if (!Target->Implements<UOSImpactReceiver>())
		{
			continue;
		}
			SecondaryHitActors.Add(Target);

		
		FCharacterSkill* Skill = GetSkillData(TEXT("Asher_Secondary"));
		if (!Skill)
			return;
		
		FOSImpactData Data = MakeImpactData(*Skill);

		IOSImpactReceiver::Execute_ReceiveImpact(Target, Data, this);
		
		// FOSImpactData Data;
		// Data.TeamSide = TeamSide;
		// Data.Direction = FVector2D(SecondaryDashDirection.X, SecondaryDashDirection.Y);
		// Data.PlayerDamage = Secondary_PlayerDamage;
		// Data.PlayerKnockbackPower = Secondary_PlayerKnockback;
		// Data.CoreKnockbackPower = Secondary_CoreKnockback;
		
	}

	SecondaryLastTraceLocation = CurrentLocation;
}

void AAsher::EndSecondaryDash()
{
	if (!HasAuthority())
	{
		return;
	}

	if (!bIsSecondary_Dashing)
	{
		return;
	}

	// DoSecondaryDashTrace();
	GetWorldTimerManager().ClearTimer(SecondaryDashTimer);

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->StopMovementImmediately();
	}

	bIsSecondary_Dashing = false;
	SecondaryDashDirection = FVector::ZeroVector;
	SecondaryHitActors.Empty();
}

UAsher_AnimInstance* AAsher::GetAsher_AnimInstance() const
{
	return Cast<UAsher_AnimInstance>(GetMesh()->GetAnimInstance());
}




void AAsher::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAsher, bIsPrimary_Attacking);
	DOREPLIFETIME(AAsher, bPrimary_SkillCoolDown);
	DOREPLIFETIME(AAsher, bSpecial_SkillCoolDown);
	DOREPLIFETIME(AAsher, bSecondary_SkillCoolDown);
	DOREPLIFETIME(AAsher, bIsSecondary_Dashing);
}

void AAsher::ServerRPC_StartCoreHit_Implementation(FVector2D SkillDir)
{
	if (bCoreHitCoolDown)
	{
		return;
	}

	CursorDir = SkillDir.GetSafeNormal();
	if (CursorDir.IsNearlyZero())
	{
		const FVector Forward = GetActorForwardVector().GetSafeNormal2D();
		CursorDir = FVector2D(Forward.X, Forward.Y);
	}

	SetActorRotation(FVector(CursorDir.X, CursorDir.Y, 0.f).Rotation());
	MulticastRPC_PlayCoreHit(CursorDir);
}

void AAsher::ServerRPC_StartPrimarySkill_Implementation(FVector2D SkillDir)
{
	if (bPrimary_SkillCoolDown || bIsPrimary_Attacking)
	{
		return;
	}

	CursorDir = SkillDir.GetSafeNormal();
	if (CursorDir.IsNearlyZero())
	{
		const FVector Forward = GetActorForwardVector().GetSafeNormal2D();
		CursorDir = FVector2D(Forward.X, Forward.Y);
	}

	bIsPrimary_Attacking = true;
	HitActors.Empty();
	SetActorRotation(FVector(CursorDir.X, CursorDir.Y, 0.f).Rotation());
	MulticastRPC_PlayPrimarySkill(CursorDir);

	DoPrimaryHit1();

	GetWorldTimerManager().SetTimer(
		PrimaryHit2Timer,
		this,
		&AAsher::DoPrimaryHit2,
		0.25f,
		false
	);

	GetWorldTimerManager().SetTimer(
		PrimaryEndTimer,
		[this]()
		{
			bIsPrimary_Attacking = false;
			Primary_SkillCool = 4.f;
		},
		0.6f,
		false
	);
}

void AAsher::ServerRPC_StartSecondarySkill_Implementation(FVector2D SkillDir)
{
	if (bSecondary_SkillCoolDown || bIsSecondary_Dashing)
	{
		return;
	}

	SecondaryDashDirection = FVector(SkillDir.X, SkillDir.Y, 0.f).GetSafeNormal();
	if (SecondaryDashDirection.IsNearlyZero())
	{
		SecondaryDashDirection = GetActorForwardVector();
		SecondaryDashDirection.Z = 0.f;
		SecondaryDashDirection.Normalize();
	}

	SetActorRotation(SecondaryDashDirection.Rotation());
	bSecondary_SkillCoolDown = true;
	SecondaryHitActors.Empty();
	MulticastRPC_PlaySecondarySkill(FVector2D(SecondaryDashDirection.X, SecondaryDashDirection.Y));
	DoSecondaryDash();
}

void AAsher::ServerRPC_StartSpecialSkill_Implementation(FVector2D SkillDir)
{
	if (bSpecial_SkillCoolDown)
	{
		return;
	}

	CursorDir = SkillDir.GetSafeNormal();
	if (CursorDir.IsNearlyZero())
	{
		const FVector Forward = GetActorForwardVector().GetSafeNormal2D();
		CursorDir = FVector2D(Forward.X, Forward.Y);
	}

	SetActorRotation(FVector(CursorDir.X, CursorDir.Y, 0.f).Rotation());
	bSpecial_SkillCoolDown = true;
	HitActors.Empty();
	MulticastRPC_PlaySpecialSkill(CursorDir);
	DoSpecialProjectile();

	GetWorld()->GetTimerManager().SetTimer(
		SpecialSkillTimer,
		[this]()
		{
			bSpecial_SkillCoolDown = false;
		},
		Special_SkillCool,
		false
	);
}

void AAsher::MulticastRPC_PlayCoreHit_Implementation(FVector2D SkillDir)
{
	const FVector Forward = FVector(SkillDir.X, SkillDir.Y, 0.f).GetSafeNormal();
	if (!Forward.IsNearlyZero())
	{
		SetActorRotation(Forward.Rotation());
	}

	if (UAsher_AnimInstance* Anim = GetAsher_AnimInstance())
	{
		Anim->PlayStrike();
	}
}

void AAsher::MulticastRPC_PlayPrimarySkill_Implementation(FVector2D SkillDir)
{
	const FVector Forward = FVector(SkillDir.X, SkillDir.Y, 0.f).GetSafeNormal();
	if (!Forward.IsNearlyZero())
	{
		SetActorRotation(Forward.Rotation());
	}

	if (UAsher_AnimInstance* Anim = GetAsher_AnimInstance())
	{
		Anim->PlayPrimary();
	}
	
	if (IsLocallyControlled() && SkillUI)
	{
		SkillUI->LoadPrim();
	}
}

void AAsher::MulticastRPC_PlaySecondarySkill_Implementation(FVector2D SkillDir)
{
	const FVector Forward = FVector(SkillDir.X, SkillDir.Y, 0.f).GetSafeNormal();
	if (!Forward.IsNearlyZero())
	{
		SetActorRotation(Forward.Rotation());
	}

	if (UAsher_AnimInstance* Anim = GetAsher_AnimInstance())
	{
		Anim->PlaySecondary();
	}
	
	if (IsLocallyControlled() && SkillUI)
	{
		SkillUI->LoadSeco();
	}
}

void AAsher::MulticastRPC_PlaySpecialSkill_Implementation(FVector2D SkillDir)
{
	const FVector Forward = FVector(SkillDir.X, SkillDir.Y, 0.f).GetSafeNormal();
	if (!Forward.IsNearlyZero())
	{
		SetActorRotation(Forward.Rotation());
	}

	if (UAsher_AnimInstance* Anim = GetAsher_AnimInstance())
	{
		Anim->PlaySpecial();
	}
	if (IsLocallyControlled() && SkillUI)
	{
		SkillUI->LoadSpec();
	}
}
