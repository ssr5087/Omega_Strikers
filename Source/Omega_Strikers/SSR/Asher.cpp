// Fill out your copyright notice in the Description page of Project Settings.


#include "Asher.h"

#include "Core/CoreBall.h"
#include "Kismet/KismetSystemLibrary.h"


// Sets default values
AAsher::AAsher()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
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

}

// Called when the game starts or when spawned
void AAsher::BeginPlay()
{
	Super::BeginPlay();
	
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
	Super::Ready_PrimarySkill();
	
	
}

void AAsher::Ready_SecondarySkill()
{
	Super::Ready_SecondarySkill();
}

void AAsher::Ready_SpecialSkill()
{
	Super::Ready_SpecialSkill();
}

void AAsher::Ready_Flip()
{
	Super::Ready_Flip();
}

void AAsher::Use_CoreHit()
{
	Super::Use_CoreHit();
}

void AAsher::Use_PrimarySkill()
{
	Super::Use_PrimarySkill();
	
	if (bPrimarySkillCoolDown)
		return;
	if (bIsPrimaryAttacking)
		return;
	
	bIsPrimaryAttacking = true;
	HitActors.Empty();
	
	// 1타 실행
	DoPrimaryHit1();
	
	// 2타 예약
	GetWorldTimerManager().SetTimer(
		PrimaryHit2Timer,
		this,
		&AAsher::DoPrimaryHit2,
		0.25f,
		false
	);
	
	// 콤보 종료
	GetWorldTimerManager().SetTimer(
		PrimaryEndTimer,
		[this]()
		{
			bIsPrimaryAttacking = false;
			PrimarySkillCool = 4.f;
		},
		0.6f,
		false
	);
}

void AAsher::Use_SecondarySkill()
{
	Super::Use_SecondarySkill();
}

void AAsher::Use_SpecialSkill()
{
	Super::Use_SpecialSkill();
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
		150.f,
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

		FOSImpactData Data;
		Data.Direction = CursorDir;
		Data.PlayerDamage = 100.f;
		Data.PlayerKnockbackPower = 600.f;

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
		200.f,
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

		float Damage;
		float Knockback;

		if (Dot > 0.8f) // 👉 중앙 판정
		{
			Damage = 200.f;
			Knockback = 1200.f;
		}
		else if (Dot > 0.3f)    // 사이드
		{
			Damage = 80.f;
			Knockback = 700.f;
		}
		else
		{
			continue; // 뒤쪽 무시
		}

		FOSImpactData Data;
		Data.Direction = CursorDir;
		Data.CoreKnockbackPower = 10000.f;
		Data.PlayerDamage = Damage;
		Data.PlayerKnockbackPower = Knockback;

		IOSImpactReceiver::Execute_ReceiveImpact(Target, Data, this);
		
	}
	HitActors.Empty();
	
	// 쿨타임
	bPrimarySkillCoolDown = true;
	FTimerHandle PrimarySkillTimer;
	GetWorld()->GetTimerManager().SetTimer(PrimarySkillTimer, [this]()->void {bPrimarySkillCoolDown = false;}, PrimarySkillCool, false);
}
