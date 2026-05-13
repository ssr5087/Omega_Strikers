// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerHUDWidget.h"

#include "HPComponent.h"
#include "HPStatusWidget.h"
#include "PlayerBase.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"

void UPlayerHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	APlayerController* pc = GetOwningPlayer();
	if (!pc) {return;}
	
	for (auto& Pair : HPWidgetMap)
	{
		APlayerBase* Player = Pair.Key;
		UHPStatusWidget* HPWidget = Pair.Value;
		
		if (!Player || !HPWidget) {continue;}
		
		const FVector WorldLocation = Player->GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);
		FVector2D ScreenPosition;
		const bool bProjected = pc->ProjectWorldLocationToScreen(WorldLocation, ScreenPosition, true);
		
		if (!bProjected)
		{
			HPWidget->SetVisibility(ESlateVisibility::Hidden);
			continue;
		}
		
		HPWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
		
		const float ViewportScale = UWidgetLayoutLibrary::GetViewportScale(this);
		ScreenPosition /= ViewportScale;
		
		UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(HPWidget->Slot);
		if (CanvasSlot)
		{
			ScreenPosition.X -= 50;
			ScreenPosition.Y -= 50;
			CanvasSlot->SetPosition(ScreenPosition);
			HPWidget->SetStaggerPercent(Player->HPComp->CurHP / Player->HPComp->MaxHP);
			HPWidget->SetLevel(Player->Level);
		}
	}
}

void UPlayerHUDWidget::RegisterPlayer(class APlayerBase* TargetPlayer)
{
	if (!TargetPlayer || !Canvas_Whole || !HPBar) {return;}
	
	// 이미 등록한 플레이어 정보 중복 등록 방지
	if (HPWidgetMap.Contains(TargetPlayer)) {return;}
	
	UHPStatusWidget* HPWidget = CreateWidget<UHPStatusWidget>(GetOwningPlayer(), HPBar);
	if (HPWidget)
	{
		Canvas_Whole->AddChild(HPWidget);
		
		UCanvasPanelSlot* slot = Cast<UCanvasPanelSlot>(Canvas_Whole->Slot);
		if (slot)
		{
			slot->SetAutoSize(true);
			slot->SetAlignment(FVector2D(0.5f, 0.5f));
		}
		
		HPWidgetMap.Add(TargetPlayer, HPWidget);
		HPWidget->SetTeamSide(TargetPlayer->TeamSide);
		HPWidget->SetStaggerPercent(TargetPlayer->HPComp->CurHP / TargetPlayer->HPComp->MaxHP);
		HPWidget->SetLevel(TargetPlayer->Level);
	}
}

void UPlayerHUDWidget::UnRegisterPlayer(class APlayerBase* TargetPlayer)
{
	
}
