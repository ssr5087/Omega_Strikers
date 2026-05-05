// Fill out your copyright notice in the Description page of Project Settings.


#include "Luna.h"

#include "HPComponent.h"
#include "InputActionValue.h"
#include "Luna_PrimaryRocket.h"
#include "Luna_SpecialRocket.h"
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
	
	// 임의 설정
	TeamSide = EOSTeam::Red;
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
		
		// 입력되고 있을 때만 방향 전환 보간 가능
		if (AimDir.IsNearlyZero())
		{
			// 애초에 보간이 진행되지 않게 off
			bIsChangingDirection = false;
			// 보간 타이머가 돌고 있어도 목표 방향과 현재 방향을 일치시키면 멈춤
			NewDir = CurDir;
			return;
		}
		
		// 방향 전환 입력했으니 반영되도록 설정
		bIsChangingDirection = true;
		
		return;
	}
	Super::PlayerMove(InputActionValue);
}

// ==================================================================
// [Core] 
// ==================================================================

void ALuna::Ready_CoreHit()
{
	Super::Ready_CoreHit();
}

void ALuna::Use_CoreHit()
{
	Super::Use_CoreHit();
}

// ==================================================================
// [Primary] 애니메이션, 쿨타임 관리, 시선 처리 / 로켓 스폰 / 시선 처리 초기화
// ==================================================================

void ALuna::Ready_PrimarySkill()
{
	
}

void ALuna::Use_PrimarySkill()
{
	// [Primary] 스폰 - 걔가 알아서 이동 - 맨 처음 충돌 객체 저장 - 값 전달 - Destroy
	
	// 현재 쿨타임 중이면 실행 안 됨
	if (bPrimarySkillCoolDown) {return;}
	
	// 애니메이션 실행 transition 세팅
	bIsProcessingPrimary = true;
	
	// 발사 방향 저장
	PrimaryDir = CursorDir;
	
	// 스킬 사용 방향을 바라보도록 설정
	GetCharacterMovement()->bOrientRotationToMovement = false;
	SetActorRotation(UKismetMathLibrary::MakeRotFromXZ(FVector(PrimaryDir.X, PrimaryDir.Y, 0), GetActorUpVector()));
	
	// 쿨타임 관리
	bPrimarySkillCoolDown = true;
	FTimerHandle PrimarySkillTimer;
	GetWorld()->GetTimerManager().SetTimer(PrimarySkillTimer, [this]()->void {bPrimarySkillCoolDown = false;}, PrimarySkillCool, false);
	
	// 애니메이션 실행 transition 처리 타이머
	FTimerHandle Primary;
	GetWorld()->GetTimerManager().SetTimer(Primary, [this]()->void {bIsProcessingPrimary = false;}, 1.f, false);
}

void ALuna::SpawnPrimaryRocket()
{
	// 스폰 트랜스폼 만들기
	FTransform LauncherTransform;
	
	// 발사 방향 (플레이어 -> 커서 방향)
	FVector PlayerLoc = FVector(GetActorLocation().X, GetActorLocation().Y, 50.f);
	FVector LaunchDir = FVector(PrimaryDir.X, PrimaryDir.Y, 0);
	FRotator SpawnRot = UKismetMathLibrary::MakeRotFromXZ(LaunchDir, GetActorUpVector());
	
	LauncherTransform.SetLocation(PlayerLoc);
	LauncherTransform.SetRotation(SpawnRot.Quaternion());
	
	GetWorld()->SpawnActor<ALuna_PrimaryRocket>(PrimaryRocketFactory, LauncherTransform);
	
	// 소환할 로켓을 변수에 담아두고 잠시 소환을 보류
	ALuna_PrimaryRocket* Rocket = GetWorld()->SpawnActorDeferred<ALuna_PrimaryRocket>(PrimaryRocketFactory, LauncherTransform);
	
	// 소환한 로켓에 Owner를 전달하고 소환 완료
	// param 처리 안하고 굳이 Deffered로 스폰해서 값을 따로 전달하는 이유는
	// 어디서 꼬인 건지 모르겠지만 아무튼 Owner 설정되기 전에 Overlap 감지부터 해서
	// 소환하자마자 Owner 무시 로직을 통과하지 못하고 Destroy 되는 것을 막기 위함
	if (Rocket)
	{
		Rocket->InitRocket(this);
		Rocket->FinishSpawning(LauncherTransform);
	}
}

void ALuna::End_PrimarySkill()
{
	GetCharacterMovement()->bOrientRotationToMovement = true;
}

// ==================================================================
// [Secondary] 애니메이션, 가속, 쿨타임 관리 / 직접 이동 / 충돌 시 데미지 처리
// + PlayerMove에서도 관련 로직 포함됨
// ==================================================================

void ALuna::Ready_SecondarySkill()
{
	
}

void ALuna::Use_SecondarySkill()
{
	// [Secondary] 직접 타고 이동 - 맨 처음 충돌 객체 저장 - 멈춤 - 저장된 객체에 값 전달
	if (bSecondarySkillCoolDown) {return;}
	
	// 즉각적으로 이동을 멈추고, 속도를 0으로 만들고, 이미 입력된 속도 벡터 값을 소모해버린 후 최대 속력 변경
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->Velocity = FVector::ZeroVector;
	GetCharacterMovement()->ConsumeInputVector();
	GetCharacterMovement()->MaxWalkSpeed = 10000.f;
	
	// 최초 이동 방향
	CurDir = FVector(CursorDir.X, CursorDir.Y, 0.0f);
	
	// 데이터 셋 불러오기
	FCharacterSkill* Skill = GetSkillData(FName(TEXT("Luna_Secondary")));
	
	// 데이터 셋 로드 실패 시 리턴
	if (!Skill) {return;}
			
	// ImpactData를 데이터 셋으로부터 계산하여 설정
	SecondaryImpactData = MakeImpactData(*Skill);
	SecondaryImpactData.Direction = FVector2D(CurDir.X, CurDir.Y);
	
	// 업데이트 함수 타이머 호출
	GetWorldTimerManager().SetTimer(MoveTimer, this, &ALuna::Update_SecondaryMove, 0.1f, true);
	
	// 애니메이션 실행 transition 세팅
	bIsProcessingSecondary = true;
	
	// 쿨타임 관리
	bSecondarySkillCoolDown = true;
	FTimerHandle SecondarySkillTimer;
	GetWorld()->GetTimerManager().SetTimer(SecondarySkillTimer, [this]()->void {bSecondarySkillCoolDown = false;}, SecondarySkillCool, false);
}

void ALuna::Update_SecondaryMove()
{
	// xy축 설정 기반 순간 이동량 추가 - MaxWalkSpeed 사용을 위한 로직
	FVector Forward = UKismetMathLibrary::GetForwardVector(FRotator(0.0f, GetControlRotation().Yaw, 0.0f));
	FVector Right = UKismetMathLibrary::GetRightVector(FRotator(0.0f, GetControlRotation().Yaw, GetControlRotation().Roll));
	AddMovementInput(Forward, CurDir.X);
	AddMovementInput(Right, CurDir.Y);
	
	// 방향 전환 입력 들어오고 있을 때만 
	if (!bIsChangingDirection) {return;}
	// 보간 실행
	CurDir = FMath::VInterpTo(CurDir, NewDir, 0.1, 3);
	CurDir.Normalize();
	
	// 방향 입력 종료 시 방향 전환 종료
	bIsChangingDirection = false;
	NewDir = CurDir;
}

void ALuna::OnDashOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 보조 스킬 처리 중일 때만 활성화
	if (!bIsProcessingSecondary) {return;}
	
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
			// 타이머, 이동 속도, 애니메이션 transition, 보간 목적 방향 초기화
			GetWorldTimerManager().ClearTimer(MoveTimer);
			GetCharacterMovement()->MaxWalkSpeed = 1000.f;
			bIsProcessingSecondary = false;
			NewDir = FVector::ZeroVector;
		}
	}
}

// ==================================================================
// [Special] 애니메이션, 가속, 쿨타임 관리 / 직접 이동 / 시선 처리 초기화
// ==================================================================

void ALuna::Ready_SpecialSkill()
{
	
}

void ALuna::Use_SpecialSkill()
{
	// [Special] 지정 위치에 z축 이동만 하는 로켓 소환
	// 바닥 좌표에 도달하는 순간 모든 적군과 코어에 방향과 충격량 각각 전달
	if (bSpecialSkillCoolDown) {return;}
	
	// 애니메이션 transition
	bIsProcessingSpecial = true;
	
	// 스폰 트랜스폼 만들기
	FTransform LauncherTransform;
	
	// 스폰 위치 (높이는 임의 설정)
	FVector LaunchDir = - UKismetMathLibrary::GetUpVector(FRotator(0.0f, GetControlRotation().Yaw, 0.0f));
	FRotator SpawnRot = UKismetMathLibrary::MakeRotFromX(LaunchDir);
	
	LauncherTransform.SetLocation(FVector(MouseCursorLoc.X, MouseCursorLoc.Y, 5000.f));
	LauncherTransform.SetRotation(SpawnRot.Quaternion());
	
	// 스폰
	ALuna_SpecialRocket* Rocket = GetWorld()->SpawnActorDeferred<ALuna_SpecialRocket>(SpecialRocketFactory, LauncherTransform);
	
	if (Rocket)
	{
		Rocket->InitRocket(Power, this, TeamSide);
		Rocket->FinishSpawning(LauncherTransform);
	}
	
	// 쿨타임 관리
	bSpecialSkillCoolDown = true;
	FTimerHandle SpecialSkillTimer;
	GetWorld()->GetTimerManager().SetTimer(SpecialSkillTimer, [this]()->void {bSpecialSkillCoolDown = false;}, SpecialSkillCool, false);

	// 애니메이션 실행 transition 처리 타이머
	FTimerHandle Special;
	GetWorld()->GetTimerManager().SetTimer(Special, [this]()->void {bIsProcessingSpecial = false;}, 1.f, false);
}

void ALuna::SpawnSpecialRocket()
{
}

// ==================================================================
// [Flip] 
// ==================================================================

void ALuna::Ready_Flip()
{
	
}

void ALuna::Use_Flip()
{
	
}



