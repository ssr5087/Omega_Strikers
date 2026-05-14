// Fill out your copyright notice in the Description page of Project Settings.


#include "OSPlayerController.h"

#include "GoalWidget.h"
#include "OSGameState.h"
#include "PlayerBase.h"
#include "PlayerHUDWidget.h"
#include "ScoreBoardWidget.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

void AOSPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	
	if (!IsLocalController()) {return;}
	
	if (!PlayerHUDWidgetClass) {return;}
	
	PlayerHUDWidget = CreateWidget<UPlayerHUDWidget>(this, PlayerHUDWidgetClass);
	
	if (PlayerHUDWidget)
	{
		PlayerHUDWidget->AddToViewport();
		
		GetWorldTimerManager().SetTimer(
					RegisterTimer,
					this,
					&AOSPlayerController::RegisterExistingPlayersToHUD,
					0.3f,
					true
				);
	}
	
	auto gs = Cast<AOSGameState>(GetWorld()->GetGameState());
	
	if (!gs) {return;}
	gs->OnScoreChanged.AddDynamic(this, &AOSPlayerController::SetScoreBoard);
	
	AddScoreBoard();
}

bool AOSPlayerController::GetMousePointOnArenaPlane(FVector& OutPoint) const
{
	// 마우스 커서의 월드 위치와 카메라 방향 벡터
	FVector WorldLocation;
	FVector WorldDirection;

	// 값을 못 받았거나 카메라 방향 벡터가 평면과 거의 평행하면 실패 처리
	if (!DeprojectMousePositionToWorld(WorldLocation, WorldDirection) || FMath::IsNearlyZero(WorldDirection.Z))
	{
		return false;
	}

	// 경기장의 z좌표(현재 0)에서 마우스의 월드 위치 z좌표를 빼고 방향벡터 값으로 나눔 -> 코어의 z좌표로 변경
	// 이 값은 음수 / 음수이므로 정상적인 경우 0보다 확실하게 큰 양수 값으로 나옴
	const float T = (50 - WorldLocation.Z) / WorldDirection.Z;

	// 카메라 뒤쪽 교차는 무효 처리 (T가 음수 나오면 뭔가 잘못된 경우, ex. 카메라가 땅 속에 숨은 상황 등)
	if (T < 0.0f)
	{
		return false;
	}

	// 마우스가 찍은 월드에서의 위치 값을 입력 값에 넣은 FVector에 반환
	OutPoint = WorldLocation + (WorldDirection * T);
	return true;
}

UPlayerHUDWidget* AOSPlayerController::GetPlayerHUDWidget() const
{
	return PlayerHUDWidget;
}

void AOSPlayerController::RegisterExistingPlayersToHUD()
{
	if (!IsValid(PlayerHUDWidget)) {return;}
	
	TArray<AActor*> FoundPlayers;

	UGameplayStatics::GetAllActorsOfClass(
		GetWorld(),
		APlayerBase::StaticClass(),
		FoundPlayers
	);

	for (AActor* Actor : FoundPlayers)
	{
		APlayerBase* TargetPlayer = Cast<APlayerBase>(Actor);

		if (TargetPlayer)
		{
			PlayerHUDWidget->RegisterPlayer(TargetPlayer);
		}
	}
}

void AOSPlayerController::AddScoreBoard()
{
	// 자기 화면에만 붙이면 됨
	if (!IsLocalController()) {return;}
	
	ScoreBoardWidget = CreateWidget<UScoreBoardWidget>(this, ScoreBoardWidgetClass);
	if (ScoreBoardWidget)
	{
		ScoreBoardWidget->AddToViewport();
		ScoreBoardWidget->SetScore(0, 0);
	}
}

void AOSPlayerController::SetScoreBoard(int32 TeamIndex, int32 NewScore)
{
	if (TeamIndex == 0) {BlueScore = NewScore;}
	else {RedScore = NewScore;}
	
	if (ScoreBoardWidget)
	{
		ScoreBoardWidget->SetScore(BlueScore, RedScore);
	}
	
	GetWorldTimerManager().SetTimer(AddWidgetTimer, this, &AOSPlayerController::AddWidget, 2.0f, false);
}

void AOSPlayerController::AddWidget()
{
	GoalWidget = CreateWidget<UGoalWidget>(GetWorld(), GoalWidgetClass);
	if (GoalWidget)
	{
		GoalWidget->AddToViewport();
		GoalWidget->PlayGoalAnimation(0);
		
		GetWorldTimerManager().SetTimer(GoalAnimTimer, this, &AOSPlayerController::RemoveWidget, 3.0f, false);
	}
}

void AOSPlayerController::RemoveWidget()
{
	GoalWidget->RemoveFromParent();
}
