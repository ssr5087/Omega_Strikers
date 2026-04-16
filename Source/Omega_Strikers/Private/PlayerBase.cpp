// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerBase.h"

#include "InputMappingContext.h"
#include "InputAction.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
APlayerBase::APlayerBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// ========= Component =========
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 1440.f, 0.f);
	
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
}

// Called when the game starts or when spawned
void APlayerBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APlayerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
	AddMovementInput(Forward, value.X * Speed);
	AddMovementInput(Right, value.Y * Speed);
}

