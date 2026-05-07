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
	
	SelectBorder->SetBrushColor(bSelected ? FLinearColor(0.95f, 0.2f, 0.48f, 1.f) : FLinearColor(0.f, 0.f, 0.f, 0.f));
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
	OnClicked.Broadcast(CharacterID);
}
