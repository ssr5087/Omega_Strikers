// Fill out your copyright notice in the Description page of Project Settings.
// 천전천승 — 캐릭터 선택 화면 구현

#include "OSCharSelectWidget.h"

#include "OSCharCardWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Omega_Strikers/Omega_Strikers.h"
#include "Omega_Strikers/SSR/CharacterStat.h"

void UOSCharSelectWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (SelectButton)
	{
		SelectButton->OnClicked.AddDynamic(this, &UOSCharSelectWidget::OnSelectClicked);
	}
	
	if (BackButton)
	{
		BackButton->OnClicked.AddDynamic(this, &UOSCharSelectWidget::OnBackClicked);
	}
	
	BuildGrid();
}

// ═══════════════════════════════════════════
//  ★ CharacterStat에서 고유 캐릭터 추출
//  Level==1 행만 필터 → Character 컬럼의 고유 이름
// ═══════════════════════════════════════════
void UOSCharSelectWidget::ExtractUniqueCharacters(TArray<FName>& OutNames, TMap<FName, FCharacterStat>& OutStats)
{
	if ( !CharacterStatTable ) return;
	
	const FString Ctx(TEXT("CharSelect"));
	TArray<FCharacterStat*> rows;
	CharacterStatTable->GetAllRows<FCharacterStat>(Ctx, rows);
	
	TSet<FName> Seen;
	
	for (const FCharacterStat* row : rows)
	{
		if (!row) continue;
		if (row->Level != 1) continue; // Lv.1 만 추출
		if (Seen.Contains(row->Character)) continue;
		
		Seen.Add(row->Character);
		OutNames.Add(row->Character);
		OutStats.Add(row->Character, *row);
	}
	
	LOG_GT(TEXT("%d명 추출"), OutNames.Num());
}

// ═══════════════════════════════════════════
//  그리드 생성
// ═══════════════════════════════════════════
void UOSCharSelectWidget::BuildGrid()
{
	if ( CardWidgetClass.Get() == nullptr || !CharacterGrid ) return;
	
	CharacterGrid->ClearChildren();
	Cards.Empty();
	StatCache.Empty();
	
	TArray<FName> names;
	ExtractUniqueCharacters(names, StatCache);
	
	int32 idx = 0;
	for (const FName& name : names)
	{
		UOSCharCardWidget* card = CreateWidget<UOSCharCardWidget>(GetOwningPlayer(), CardWidgetClass);
		if ( !card ) continue;
		
		card->Setup(name);
		card->OnClicked.AddDynamic(this, &UOSCharSelectWidget::OnCardClicked);
		
		CharacterGrid->AddChildToUniformGrid(card, idx / Columns, idx % Columns);
		Cards.Add(card);
		idx++;
	}
	
	// 첫 캐릭터 자동 선택
	if (names.Num() > 0)
	{
		OnCardClicked(names[0]);
	}
}

// ═══════════════════════════════════════════
//  프리뷰 갱신 (이름 + Lv.1 스탯)
// ═══════════════════════════════════════════
void UOSCharSelectWidget::UpdatePreview(FName CharacterID)
{
	// 이름
	if (PreviewName) PreviewName->SetText(FText::FromName(CharacterID));
	
	// Lv.1 스탯
	if (const FCharacterStat* stat = StatCache.Find(CharacterID))
	{
		if (StatHPText) StatHPText->SetText(FText::FromString(FString::Printf(TEXT("HP %.0f"), stat->MaxHP)));
		
		if (StatPowerText) StatPowerText->SetText(FText::FromString(FString::Printf(TEXT("공격력 %.0f"), stat->Power)));
		
		if (StatSpeedText) StatSpeedText->SetText(FText::FromString(FString::Printf(TEXT("속도 %.0f"), stat->Speed)));
	}
	
	// TODO: 전신 sprite 준비되면 PreviewImage 갱신
}

void UOSCharSelectWidget::ClearSelections()
{
	for (UOSCharCardWidget* card : Cards)
	{
		if (card) card->SetSelected(false);
	}
}

// ═══════════════════════════════════════════
//  카드 클릭
// ═══════════════════════════════════════════
void UOSCharSelectWidget::OnCardClicked(FName CharacterID)
{
	SelectedID = CharacterID;
	ClearSelections();
	for (UOSCharCardWidget* card : Cards)
	{
		if (card && card->GetCharacterID() == CharacterID)
		{
			card->SetSelected(true);
			break;
		}
	}
	
	UpdatePreview(CharacterID);
}

// ═══════════════════════════════════════════
//  선택 확정 / 뒤로
// ═══════════════════════════════════════════
void UOSCharSelectWidget::OnSelectClicked()
{
	if (SelectedID.IsNone()) return;
	
	OnConfirmed.Broadcast(SelectedID);
	RemoveFromParent();
}

void UOSCharSelectWidget::OnBackClicked()
{
	RemoveFromParent();
}
