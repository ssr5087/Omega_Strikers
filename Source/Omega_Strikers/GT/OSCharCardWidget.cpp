// Fill out your copyright notice in the Description page of Project Settings.
// 천전천승 — 캐릭터 카드 위젯 구현

#include "OSCharCardWidget.h"

#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UOSCharCardWidget::Setup(FName InCharacterID, UTexture2D* InPortrait)
{
	CharacterID = InCharacterID;
	
	if (NameText)
	{
		NameText->SetText(FText::FromName(CharacterID));
	}
	
	if (SameTeamNameText)
	{
		SameTeamNameText->SetText(FText::FromString(""));
		SameTeamNameText->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	if (OtherTeamNameText)
	{
		OtherTeamNameText->SetText(FText::FromString(""));
		OtherTeamNameText->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	// 초상화 텍스처 적용
	if (PortraitImage != nullptr && InPortrait != nullptr)
	{
		PortraitImage->SetBrushFromTexture(InPortrait);
	}
	
	SetSelected(false);
}

void UOSCharCardWidget::SetSelected(bool bSelected)
{
	if (!SelectBorder) return;
	
	// 잠긴 상태면 Selected 색상 덮어쓰지 않음
	if ( bIsLocked ) return;
	
	SelectBorder->SetBrushColor(bSelected ? FLinearColor(0.95f, 0.2f, 0.48f, 1.f) : FLinearColor(0.f, 0.f, 0.f, 0.f));
}

void UOSCharCardWidget::SetLocked(bool bLocked, const FString& SameTeamName, const FString& OtherTeamName)
{
	if (!CardButton || !SelectBorder) return;

	if (bLocked)
	{
		// 회색 테두리 + 반투명 + 클릭 불가
		SelectBorder->SetBrushColor(FLinearColor(0.4f, 0.4f, 0.4f, 0.8f));
		CardButton->SetIsEnabled(false);
		if (PortraitImage) PortraitImage->SetColorAndOpacity(FLinearColor(0.3f, 0.3f, 0.3f, 0.6f));
		// 같은 팀 확정자 이름 -> 메인 NameText에 표시
		if (SameTeamNameText && !SameTeamName.IsEmpty())
		{
			SameTeamNameText->SetText(FText::FromString(SameTeamName));
			SameTeamNameText->SetVisibility(ESlateVisibility::Visible);
		}
	}
	else
	{
		// 원복
		CardButton->SetIsEnabled(true);
		if (PortraitImage) PortraitImage->SetColorAndOpacity(FLinearColor::White);
		if (SameTeamNameText)
		{
			SameTeamNameText->SetText(FText::FromString(""));
			SameTeamNameText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	
	// 다른 팀 확정자 이름 → OtherTeamNameText에 표시 (잠금 여부 무관)
	if (OtherTeamNameText)
	{
		if (!OtherTeamName.IsEmpty())
		{
			OtherTeamNameText->SetText(FText::FromString(OtherTeamName));
			OtherTeamNameText->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			OtherTeamNameText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UOSCharCardWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (CardButton)
	{
		CardButton->OnClicked.AddDynamic(this, &ThisClass::HandleClicked);
	}
}

void UOSCharCardWidget::HandleClicked()
{
	// 잠긴 카드는 클릭 무시
	if ( bIsLocked ) return;
	
	OnClicked.Broadcast(CharacterID);
}
