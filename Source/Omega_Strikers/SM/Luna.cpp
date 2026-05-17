// Fill out your copyright notice in the Description page of Project Settings.


#include "Luna.h"

#include "SkillIndicatorBase.h"
#include "HPComponent.h"
#include "InputActionValue.h"
#include "Luna_PrimaryRocket.h"
#include "Luna_SpecialRocket.h"
#include "Blueprint/UserWidget.h"
#include "LunaSkillCool.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Omega_Strikers/Omega_Strikers.h"


// Sets default values
ALuna::ALuna()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	SetNetUpdateFrequency(100.f);
	SetMinNetUpdateFrequency(60.f);
	
	// ================= Component =================
	
	// 캡슐 컴포넌트 크기 조정
	GetCapsuleComponent()->SetCapsuleHalfHeight(165.0f);
	GetCapsuleComponent()->SetCapsuleRadius(102.0f);
	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &ALuna::OnDashHit);
	
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

	// ConstructorHelpers::FClassFinder<ASkillIndicatorBase> TempPrimaryIndi(TEXT("/Game/SM/Blueprints/Luna/SkillIndicator/BP_LunaIndiPrimary"));
	// if (TempPrimaryIndi.Succeeded())
	// {
	// 	LunaPrimaryIndicatorClass = TempPrimaryIndi.Class;
	// 	LunaSecondaryIndicatorClass = TempPrimaryIndi.Class;
	// 	LunaSpecialIndicatorClass = TempPrimaryIndi.Class;
	// }
}

// Called when the game starts or when spawned
void ALuna::BeginPlay()
{
	Super::BeginPlay();
	
	// 내가 조작 중인 캐릭터가 루나일 때만 UI 붙이기
	if (IsLocallyControlled())
	{
		LOG_SM_W(TEXT("나 얘 조작 중인데"));
		// UI 붙이기
		skillUI = CreateWidget<ULunaSkillCool>(GetWorld(), CoolTimeUI);
		if (skillUI)
		{
			LOG_SM_W(TEXT("붙었냐?"));
			skillUI->AddToViewport();
		}
	}
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

// 스킬 사거리 표시
void ALuna::ConfigureSkillIndicator(ESkillType SkillType, ASkillIndicatorBase* Indicator)
{
	Super::ConfigureSkillIndicator(SkillType, Indicator);

	if (!Indicator)
	{
		return;
	}

	float IndicatorRange = 0.f;
	TSubclassOf<ASkillIndicatorBase> IndicatorClass = nullptr;

	switch (SkillType)
	{
	case ESkillType::Primary:
		IndicatorRange = PrimaryRange;
		break;
	case ESkillType::Secondary:
		IndicatorRange = 700.f;
		break;
	case ESkillType::Special:
		IndicatorRange = 5000.f;
		break;
	default:
		return;
	}

	Indicator->SetIndicatorRange(IndicatorRange);
}

void ALuna::PlayerMove(const struct FInputActionValue& InputActionValue)
{
	if (bIsProcessingSecondary)
	{
		// 키보드 입력 값 정규화 및 속도 곱 전처리
		FVector2D AimDir = InputActionValue.Get<FVector2D>();

		ServerRPC_UpdateSecondaryDirection(AimDir);
		
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
	
	if (IsLocallyControlled() && skillUI)
	{
		skillUI->LoadCore();
	}
}

// ==================================================================
// [Primary] 애니메이션, 쿨타임 관리, 시선 처리 / 로켓 스폰 / 시선 처리 초기화
// ==================================================================

void ALuna::Ready_PrimarySkill()
{
	if (bPrimarySkillCoolDown)
	{
		return;
	}

	Super::Ready_PrimarySkill();
	ShowSkillIndicator(PrimaryIndicatorClass, ESkillType::Primary);
}

void ALuna::Use_PrimarySkill()
{
	Super::Use_PrimarySkill();
	// 입력되면 플레이어 -> 마우스 커서 방향 벡터만 매개변수로 서버 RPC 전달
	ServerRPC_StartPrimarySkill(CursorDir);
}

void ALuna::SpawnPrimaryRocket()
{
	// 서버 & 클라 둘 다 노티파이를 타고 옴. but 서버에서만 실행
	if (!HasAuthority()) {return;}
	
	// 스폰 트랜스폼 만들기
	FTransform LauncherTransform;
	
	// 발사 방향 (플레이어 -> 커서 방향)
	FVector PlayerLoc = FVector(GetActorLocation().X, GetActorLocation().Y, 50.f);
	FVector LaunchDir = FVector(PrimaryDir.X, PrimaryDir.Y, 0);
	FRotator SpawnRot = UKismetMathLibrary::MakeRotFromXZ(LaunchDir, GetActorUpVector());
	
	LauncherTransform.SetLocation(PlayerLoc);
	LauncherTransform.SetRotation(SpawnRot.Quaternion());
	
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

void ALuna::Primary_Sound()
{
	UGameplayStatics::PlaySound2D(this, PrimarySFX);
}

void ALuna::End_PrimarySkill()
{
	// 서버 & 클라 둘 다 노티파이를 타고 옴
	GetCharacterMovement()->bOrientRotationToMovement = true;
}

// ==================================================================
// [Secondary] 애니메이션, 가속, 쿨타임 관리 / 직접 이동 / 충돌 시 데미지 처리
// + PlayerMove에서도 관련 로직 포함됨
// ==================================================================

void ALuna::Ready_SecondarySkill()
{
	if (bSecondarySkillCoolDown)
	{
		return;
	}
	Super::Ready_SecondarySkill();

	ShowSkillIndicator(SecondaryIndicatorClass, ESkillType::Secondary);
}

void ALuna::Use_SecondarySkill()
{
	Super::Use_SecondarySkill();
	// 소유 캐릭터에서만 처리
	if (!IsLocallyControlled()) {return;}
	
	// 그냥 동기화로 처리함 RPC로 여기저기서 하려니까 너무 복잡함
	// // 로컬 입력 분기용으로 즉시 켜준다
	// bIsProcessingSecondary = true;

	// 로컬에서도 최초 방향은 세팅해둔다
	CurDir = FVector(CursorDir.X, CursorDir.Y, 0.0f);
	CurDir.Normalize();

	NewDir = CurDir;
	bIsChangingDirection = false;
	
	ServerRPC_StartSecondarySkill(CursorDir);
}

void ALuna::Secondary_Sound()
{
	UGameplayStatics::PlaySound2D(this, SecondarySFX);
}

void ALuna::Update_SecondaryMove()
{
	// 서버에서만 처리(서버 RPC에서 타이머 호출 중임)
	if (!HasAuthority()) {return;}
	
	// 방향 전환 입력이 없으면 Velocity를 건드리지 않는다
	if (!bIsChangingDirection) { return; }
	
	// 보간 실행
	CurDir = FMath::VInterpTo(CurDir, NewDir, 0.01f, 8);
	CurDir.Normalize();
	
	// 방향 입력 종료 시 방향 전환 종료
	bIsChangingDirection = false;
	NewDir = CurDir;
	
	// 방향 지정
	FVector MoveDir = CurDir;
	MoveDir.Z = 0.0f;
	MoveDir.Normalize();
	
	// 위치를 직접 바꾸지 않고 CharacterMovement 속도를 설정
	GetCharacterMovement()->Velocity = MoveDir * 10000;
}

void ALuna::OnDashOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 서버에서만 충돌 처리
	if (!HasAuthority()) {return;}
	
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
		if (OtherActor->GetName().Contains("Arena") || OtherComp->GetName().Contains("Wall"))
		{
			bIsSuccess = true;
		}
		
		// 앞에서 초기화 활성화 되었으면 실행 x
		// 충돌 대상이 인터페이스 함수 구현했다면
		if (!bIsSuccess && OtherActor->Implements<UOSImpactReceiver>())
		{
			// 현재 진행 방향으로 넉백
			SecondaryImpactData.Direction = FVector2D(CurDir.X, CurDir.Y).GetSafeNormal();
			// 그 대상에 대해 인터페이스 함수를 실행해라
			bIsSuccess = Execute_ReceiveImpact(OtherActor, SecondaryImpactData, this);
		}
		
		// 성공하면 초기화
		if (bIsSuccess)
		{
			// 타이머, 이동 속도, 애니메이션 transition, 보간 목적 방향 초기화
			GetWorldTimerManager().ClearTimer(MoveTimer);

			GetCharacterMovement()->StopMovementImmediately();
			GetCharacterMovement()->Velocity = FVector::ZeroVector;
			GetCharacterMovement()->ConsumeInputVector();
			GetCharacterMovement()->MaxWalkSpeed = 1000.f;
			GetCharacterMovement()->BrakingFrictionFactor = 2.0f;
			GetCharacterMovement()->BrakingDecelerationWalking = 2048.0f;

			bSecondaryAnimTrans = false;
			bIsProcessingSecondary = false;
			bIsChangingDirection = false;
			CurDir = FVector::ZeroVector;
			NewDir = FVector::ZeroVector;
			
			MulticastRPC_EndSecondarySkill();
		}
	}
}

// 벽 충돌도 조건으로 넣어서 방어적으로 처리
void ALuna::OnDashHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// 서버에서만 충돌 처리
	if (!HasAuthority()) {return;}
	
	// 보조 스킬 처리 중일 때만 활성화
	if (!bIsProcessingSecondary) {return;}
	
	// 자기 자신 충돌 무시
	if (OtherActor == this) {return;}
	
	// 타이머, 이동 속도, 애니메이션 transition, 보간 목적 방향 초기화
	GetWorldTimerManager().ClearTimer(MoveTimer);

	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->Velocity = FVector::ZeroVector;
	GetCharacterMovement()->ConsumeInputVector();
	GetCharacterMovement()->MaxWalkSpeed = 1000.f;
	GetCharacterMovement()->BrakingFrictionFactor = 2.0f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2048.0f;

	bSecondaryAnimTrans = false;
	bIsProcessingSecondary = false;
	bIsChangingDirection = false;
	CurDir = FVector::ZeroVector;
	NewDir = FVector::ZeroVector;
			
	MulticastRPC_EndSecondarySkill();
}

// ==================================================================
// [Special] 애니메이션, 가속, 쿨타임 관리 / 직접 이동 / 시선 처리 초기화
// ==================================================================

void ALuna::Ready_SpecialSkill()
{
	if (bSpecialSkillCoolDown)
	{
		return;
	}

	Super::Ready_SpecialSkill();
	ShowSkillIndicator(SpecialIndicatorClass, ESkillType::Special);
}

void ALuna::Use_SpecialSkill()
{
	Super::Use_SpecialSkill();
	// 입력되면 마우스 커서 위치 벡터만 매개변수로 서버 RPC 전달
	ServerRPC_StartSpecialSkill(MouseCursorLoc);	
}

void ALuna::SpawnSpecialRocket()
{
	// 서버 & 클라 둘 다 노티파이를 타고 옴. but 서버에서만 실행
	if (!HasAuthority()) {return;}
	
	// 스폰 트랜스폼 만들기
	FTransform LauncherTransform;
	
	// 스폰 위치 (높이는 임의 설정)
	FVector LaunchDir = - UKismetMathLibrary::GetUpVector(FRotator(0.0f, GetControlRotation().Yaw, 0.0f));
	FRotator SpawnRot = UKismetMathLibrary::MakeRotFromX(LaunchDir);
	
	LauncherTransform.SetLocation(FVector(SpecialLoc.X, SpecialLoc.Y, 5000.f));
	LauncherTransform.SetRotation(SpawnRot.Quaternion());
	
	// 스폰
	ALuna_SpecialRocket* Rocket = GetWorld()->SpawnActorDeferred<ALuna_SpecialRocket>(SpecialRocketFactory, LauncherTransform);
	
	if (Rocket)
	{
		Rocket->InitRocket(this);
		Rocket->FinishSpawning(LauncherTransform);
	}
}

void ALuna::Special_Sound()
{
	UGameplayStatics::PlaySound2D(this, SpecialSFX);
}

void ALuna::End_SpecialSkill()
{
	// 서버 & 클라 둘 다 노티파이를 타고 옴. but 서버에서만 실행
	GetCharacterMovement()->bOrientRotationToMovement = true;
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


// ==================================================================
// [Network] 동기화 변수 등록 / OnRep 함수 / RPC 함수
// ==================================================================

void ALuna::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ALuna, bPrimaryAnimTrans);
	DOREPLIFETIME(ALuna, bSecondaryAnimTrans);
	DOREPLIFETIME(ALuna, bSpecialAnimTrans);
	DOREPLIFETIME(ALuna, bIsProcessingSecondary);
}

// ----------------- Primary -----------------

void ALuna::ServerRPC_StartPrimarySkill_Implementation(FVector2D SpawnDir)
{
	// 현재 쿨타임 중이면 실행 안 됨
	if (bPrimarySkillCoolDown) {return;}
	
	// 애니메이션 실행 transition 세팅
	bPrimaryAnimTrans = true;
	
	// 사운드 재생
	Primary_Sound();
	
	// 발사 방향 저장
	PrimaryDir = SpawnDir;
	
	// 스킬 사용 방향을 바라보도록 설정 - 클라에서도 멀티캐스트
	GetCharacterMovement()->bOrientRotationToMovement = false;
	SetActorRotation(UKismetMathLibrary::MakeRotFromXZ(FVector(PrimaryDir.X, PrimaryDir.Y, 0), GetActorUpVector()));
	MulticastRPC_StartPrimaryLook();
	
	// 쿨타임 관리
	bPrimarySkillCoolDown = true;
	FTimerHandle PrimarySkillTimer;
	GetWorld()->GetTimerManager().SetTimer(PrimarySkillTimer, [this]()->void {bPrimarySkillCoolDown = false;}, PrimarySkillCool, false);
	
	// 애니메이션 실행 transition 처리 타이머
	FTimerHandle Primary;
	GetWorld()->GetTimerManager().SetTimer(Primary, [this]()->void {bPrimaryAnimTrans = false;}, 1.f, false);
}

void ALuna::MulticastRPC_StartPrimaryLook_Implementation()
{
	if (IsLocallyControlled() && skillUI)
	{
		skillUI->LoadPrim();
	}
	GetCharacterMovement()->bOrientRotationToMovement = false;
	SetActorRotation(UKismetMathLibrary::MakeRotFromXZ(FVector(PrimaryDir.X, PrimaryDir.Y, 0), GetActorUpVector()));
	
	// 사운드 재생
	Primary_Sound();
}

// ---------------- Secondary ----------------

void ALuna::ServerRPC_StartSecondarySkill_Implementation(FVector2D StartDir)
{
	// [Secondary] 직접 타고 이동 - 맨 처음 충돌 객체 저장 - 멈춤 - 저장된 객체에 값 전달
	if (bSecondarySkillCoolDown) {return;}
	
	// 사운드 재생
	Secondary_Sound();
	
	// 즉각적으로 이동을 멈추고, 속도를 0으로 만들고, 이미 입력된 속도 벡터 값을 소모해버린 후 최대 속력 변경
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->Velocity = FVector::ZeroVector;
	GetCharacterMovement()->ConsumeInputVector();
	GetCharacterMovement()->MaxWalkSpeed = 10000.f;
	
	// 최초 이동 방향
	CurDir = FVector(StartDir.X, StartDir.Y, 0.0f);
	CurDir.Normalize();

	const float DashSpeed = 10000.f;
	GetCharacterMovement()->Velocity = CurDir * DashSpeed;
	
	// 마찰 줄이고 이동 계속 유지하게 하기
	GetCharacterMovement()->BrakingFrictionFactor = 0.0f;
	GetCharacterMovement()->BrakingDecelerationWalking = 0.0f;
	
	// 데이터 셋 불러오기
	FCharacterSkill* Skill = GetSkillData(FName(TEXT("Luna_Secondary")));
	
	// 데이터 셋 로드 실패 시 리턴
	if (!Skill) {return;}
			
	// ImpactData를 데이터 셋으로부터 계산하여 설정
	SecondaryImpactData = MakeImpactData(*Skill);
	
	// 업데이트 함수 타이머 호출
	GetWorldTimerManager().SetTimer(MoveTimer, this, &ALuna::Update_SecondaryMove, 0.01f, true);
	
	// 애니메이션 실행 transition 세팅
	bSecondaryAnimTrans = true;
	
	// 처리 중 변수 세팅
	bIsProcessingSecondary = true;
	
	// 쿨타임 UI 애니메이션 재생
	MulticastRPC_StartSecondaryCool();
	
	// 쿨타임 관리
	bSecondarySkillCoolDown = true;
	FTimerHandle SecondarySkillTimer;
	GetWorld()->GetTimerManager().SetTimer(SecondarySkillTimer, [this]()->void {bSecondarySkillCoolDown = false;}, SecondarySkillCool, false);
}

void ALuna::MulticastRPC_StartSecondaryCool_Implementation()
{
	if (IsLocallyControlled() && skillUI)
	{
		skillUI->LoadSeco();
	}
	
	// 사운드 재생
	Secondary_Sound();
}

void ALuna::ServerRPC_UpdateSecondaryDirection_Implementation(FVector2D AimDir)
{
	// 혹시 세컨더리 실행 중이 맞는지 한 번 더 확인
	if (!bIsProcessingSecondary) {return;}
	
	// 방향을 3D 벡터로 변환
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
}

void ALuna::MulticastRPC_EndSecondarySkill_Implementation()
{
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->Velocity = FVector::ZeroVector;
	GetCharacterMovement()->ConsumeInputVector();
	GetCharacterMovement()->MaxWalkSpeed = 1000.f;
	GetCharacterMovement()->BrakingFrictionFactor = 2.0f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2048.0f;

	bSecondaryAnimTrans = false;
	bIsProcessingSecondary = false;
	bIsChangingDirection = false;
	CurDir = FVector::ZeroVector;
	NewDir = FVector::ZeroVector;
}

// ----------------- Special -----------------	

void ALuna::ServerRPC_StartSpecialSkill_Implementation(FVector SpawnLoc)
{
	// 쿨타임 체크
	if (bSpecialSkillCoolDown) {return;}
	
	// 애니메이션 transition
	bSpecialAnimTrans = true;
	
	// 사운드 재생
	Special_Sound();
	
	// 발사 위치 지정
	SpecialLoc = SpawnLoc;
	
	// 스킬 사용 방향을 바라보도록 설정 - 클라에서도 멀티캐스트
	GetCharacterMovement()->bOrientRotationToMovement = false;
	SetActorRotation(UKismetMathLibrary::MakeRotFromXZ(FVector(CursorDir.X, CursorDir.Y, 0), GetActorUpVector()));
	MulticastRPC_StartSpecialLook();
	
	// 쿨타임 관리
	bSpecialSkillCoolDown = true;
	FTimerHandle SpecialSkillTimer;
	GetWorld()->GetTimerManager().SetTimer(SpecialSkillTimer, [this]()->void {bSpecialSkillCoolDown = false;}, SpecialSkillCool, false);

	// 애니메이션 실행 transition 처리 타이머
	FTimerHandle Special;
	GetWorld()->GetTimerManager().SetTimer(Special, [this]()->void {bSpecialAnimTrans = false;}, 1.f, false);
}

void ALuna::MulticastRPC_StartSpecialLook_Implementation()
{
	if (IsLocallyControlled() && skillUI)
	{
		skillUI->LoadSpec();
	}
	GetCharacterMovement()->bOrientRotationToMovement = false;
	SetActorRotation(UKismetMathLibrary::MakeRotFromXZ(FVector(CursorDir.X, CursorDir.Y, 0), GetActorUpVector()));

	// 사운드 재생
	Special_Sound();
}
