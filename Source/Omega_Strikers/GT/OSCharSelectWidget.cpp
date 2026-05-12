// Fill out your copyright notice in the Description page of Project Settings.
// 천전천승 — 캐릭터 선택 화면 구현

#include "OSCharSelectWidget.h"

#include "OSCharCardWidget.h"
#include "OSCharSelectGameMode.h"
#include "OSCharSelectGameState.h"
#include "OSPlayerState.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Omega_Strikers/Omega_Strikers.h"
#include "Omega_Strikers/SSR/CharacterStat.h"

void UOSCharSelectWidget::NativeConstruct()
{
	Super::NativeConstruct();

	GetWorld()->GetTimerManager().ClearTimer(BindTimerHandle);
	
	// UI 전용 입력 모드로 전환 - 마우스는 위젯에서만 동작
	if (APlayerController* pc = GetOwningPlayer())
	{
		pc->SetInputMode(FInputModeUIOnly().SetWidgetToFocus(TakeWidget()));
		pc->SetShowMouseCursor(true);
	}
	
	if (BackButton)
	{
		BackButton->OnClicked.AddDynamic(this, &UOSCharSelectWidget::OnBackClicked);
	}
	
	if (ConfirmButton)
	{
		ConfirmButton->OnClicked.AddDynamic(this, &UOSCharSelectWidget::OnConfirmClicked);
	}	
	
	if (CancelButton)
	{
		CancelButton->OnClicked.AddDynamic(this, &UOSCharSelectWidget::OnCancelClicked);
		CancelButton->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	// ★ 게임 시작 버튼 — 호스트(서버)에게만 표시
	if (StartGameButton)
	{
		StartGameButton->OnClicked.AddDynamic(this, &UOSCharSelectWidget::OnStartGameClicked);
		
		bool bIsHost = (GetWorld()->GetAuthGameMode() != nullptr);
		StartGameButton->SetVisibility(bIsHost ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		StartGameButton->SetIsEnabled(false);
	}
	
	BuildGrid();
	
	// GameState 구독
	CachedGameState = GetWorld()->GetGameState<AOSCharSelectGameState>();
	if ( CachedGameState )
	{
		CachedGameState->OnCharSelectListUpdated.AddDynamic(this, &UOSCharSelectWidget::OnCharSelectListUpdated);
	}
	
	TryBindPlayerState();
	
	// 초기 UI 반영
	RefreshCardStates();
	UpdateButtonStates();
	UpdateStartGameButton();
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
//  ★ 이름 규칙으로 텍스처 자동 로드
//
//  ★ TMap 매핑으로 텍스처 로드
//
//  Suffix="Portrait"  → CloseUp 폴더  → T_UI_Portrait_CloseUp_{내부명}
//  Suffix="FullBody"  → Full 폴더     → T_UI_Portrait_Full_{내부명}
// ═══════════════════════════════════════════
UTexture2D* UOSCharSelectWidget::LoadCharIconTexture(FName CharacterID, const FString& Suffix)
{
	const FName* internalName = CharToTextureName.Find(CharacterID);
	if (internalName == nullptr)
	{
		LOG_GT_W(TEXT("CharSelect: 매핑 없음 → %s"), *CharacterID.ToString());
		return nullptr;
	}
 
	// Suffix에 따라 폴더와 파일명 패턴 결정
	FString basePath;
	FString prefix;
 
	if (Suffix == TEXT("Portrait"))
	{
		basePath = CloseUpPath;
		prefix = TEXT("T_UI_Portrait_CloseUp");
	}
	else
	{
		basePath = FullPath;
		prefix = TEXT("T_UI_FullCharacter");
	}
 
	const FString fileName = FString::Printf(TEXT("%s_%s"), *prefix, *internalName->ToString());
	const FString assetPath = FString::Printf(TEXT("%s/%s.%s"), *basePath, *fileName, *fileName);
 
	UTexture2D* tex = LoadObject<UTexture2D>(nullptr, *assetPath);
 
	if (tex == nullptr)
	{
		LOG_GT_W(TEXT("CharSelect: 텍스처 없음 → %s"), *assetPath);
	}
 
	return tex;
}

// ═══════════════════════════════════════════
//  그리드 생성
// ═══════════════════════════════════════════
void UOSCharSelectWidget::BuildGrid()
{
	if ( CardWidgetClass.Get() == nullptr || !CharacterGrid ) return;
	
	CharacterGrid->ClearChildren();
	CardWidgets.Empty();
	StatCache.Empty();
	
	TArray<FName> names;
	ExtractUniqueCharacters(names, StatCache);
	
	int32 idx = 0;
	for (const FName& name : names)
	{
		UOSCharCardWidget* card = CreateWidget<UOSCharCardWidget>(GetOwningPlayer(), CardWidgetClass);
		if ( !card ) continue;
		
		// ★ 초상화 자동 로드
		UTexture2D* portrait = LoadCharIconTexture(name, TEXT("Portrait"));
		
		card->Setup(name, portrait);
		card->OnClicked.AddDynamic(this, &UOSCharSelectWidget::OnCardClicked);
		
		CharacterGrid->AddChildToUniformGrid(card, idx / Columns, idx % Columns);
		CardWidgets.Add(card);
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
	// ★ 전신 이미지 자동 로드
	if (PreviewImage != nullptr)
	{
		UTexture2D* fullBody = LoadCharIconTexture(CharacterID, TEXT("FullBody"));
		if (fullBody != nullptr)
		{
			PreviewImage->SetBrushFromTexture(fullBody);
			PreviewImage->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			PreviewImage->SetVisibility(ESlateVisibility::Hidden);
		}
	}
	
	// 이름
	if (PreviewName) PreviewName->SetText(FText::FromName(CharacterID));
	
	// Lv.1 스탯
	if (const FCharacterStat* stat = StatCache.Find(CharacterID))
	{
		if (StatHPText) StatHPText->SetText(FText::FromString(FString::Printf(TEXT("HP %.0f"), stat->MaxHP)));
		
		if (StatPowerText) StatPowerText->SetText(FText::FromString(FString::Printf(TEXT("공격력 %.0f"), stat->Power)));
		
		if (StatSpeedText) StatSpeedText->SetText(FText::FromString(FString::Printf(TEXT("속도 %.0f"), stat->Speed)));
	}
}

void UOSCharSelectWidget::ClearSelections()
{
	for (UOSCharCardWidget* card : CardWidgets)
	{
		if (card) card->SetSelected(false);
	}
}

// ═══════════════════════════════════════════
//  카드 클릭 (네트워크 연동)
// ═══════════════════════════════════════════
void UOSCharSelectWidget::OnCardClicked(FName CharacterID)
{
	APlayerController* pc = GetOwningPlayer();
	if ( !pc ) return;
	
	AOSPlayerState* ps = Cast<AOSPlayerState>(pc->PlayerState);
	if (!ps) return;

	// ── 확정 상태면 무시 ──
	if (ps->IsCharacterConfirmed()) return;
	
	// ── 잠긴 캐릭터 무시 ──
	if (CachedGameState && CachedGameState->IsCharacterLocked(CharacterID)) return;

	// ── 로컬 UI 갱신 (낙관적) ── 
	SelectedID = CharacterID;
	ClearSelections();
	for (UOSCharCardWidget* card : CardWidgets)
	{
		if (card && card->GetCharacterID() == CharacterID)
		{
			card->SetSelected(true);
			break;
		}
	}
	UpdatePreview(CharacterID);
	
	// ── 서버에 선택 요청 ──
	ps->Server_RequestSelectCharacter(CharacterID);

	UpdateButtonStates();
}

// ═══════════════════════════════════════════
//  선택 확정 / 뒤로 / 확정 취소
// ═══════════════════════════════════════════
void UOSCharSelectWidget::OnBackClicked()
{
	RemoveFromParent();
}

void UOSCharSelectWidget::NativeDestruct()
{
	GetWorld()->GetTimerManager().ClearTimer(BindTimerHandle);
	
	if ( CachedGameState )
	{
		CachedGameState->OnCharSelectListUpdated.RemoveDynamic(this, &UOSCharSelectWidget::OnCharSelectListUpdated);
	}
	
	Super::NativeDestruct();
}

// ═══════════════════════════════════════════
//  네트워크 콜백
// ═══════════════════════════════════════════
void UOSCharSelectWidget::OnCharSelectListUpdated()
{
	// GameState의 선택 목록이 변경됨 -> 카드 UI 갱신
	RefreshCardStates();
	UpdateButtonStates();
	UpdateStartGameButton();
}

void UOSCharSelectWidget::RefreshCardStates()
{
	if ( !CachedGameState ) return;
	
	APlayerController* pc = GetOwningPlayer();
	if ( !pc ) return;
	
	int32 myPID = -1;
	if (AOSPlayerState* ps = Cast<AOSPlayerState>(pc->PlayerState)) myPID = ps->GetPlayerId();
	
	for (UOSCharCardWidget* card : CardWidgets)
	{
		if ( !card ) continue;
		
		FName cid = card->GetCharacterID();
		
		bool bLockedByOther = false;
		bool bLockedByMe = false;
		bool bSelectedByMe = (cid == SelectedID);
		FString LockerName;
		
		for (const FOSCharSelectEntry& entry : CachedGameState->CharSelectList)
		{
			if (entry.CharacterID == cid && entry.bConfirmed)
			{
				if (entry.PlayerIndex == myPID) bLockedByMe = true;
				else
				{
					bLockedByOther = true;
					LockerName = entry.PlayerName;
				}
				break;
			}
		}
		
		// 카드 비주얼 업데이트
		if ( bLockedByOther )
		{
			card->SetSelected(false);
			card->SetLocked(true, LockerName);
		}
		else if ( bLockedByMe )
		{
			card->SetSelected(true);
			card->SetLocked(false);  // 내가 확정 — 골드 하이라이트
		}
		else if ( bSelectedByMe )
		{
			card->SetSelected(true);
			card->SetLocked(false);
		}
		else
		{
			card->SetSelected(false);
			card->SetLocked(false);
		}
	}
}

void UOSCharSelectWidget::OnMySelectRejected(FName CharacterID, const FString& Reason)
{
	// 선택 거부 -> 선택 해제 
	SelectedID = NAME_None;
	ClearSelections();
	RefreshCardStates();
	UpdateButtonStates();
	
	// 상태 메시지 표시
	if ( StatusText )
	{
		StatusText->SetText(FText::FromString(Reason));
		StatusText->SetColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.2f, 0.2f, 1.f)));
		StatusText->SetVisibility(ESlateVisibility::Visible);
	}
}

void UOSCharSelectWidget::OnConfirmClicked()
{
	if (SelectedID.IsNone()) return;

	// ── 서버에 확정 요청 ──
	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;
	AOSPlayerState* PS = Cast<AOSPlayerState>(PC->PlayerState);
	if (!PS) return;
	PS->Server_RequestConfirmCharacter();
	OnConfirmed.Broadcast(SelectedID);

	if (ConfirmButtonText) ConfirmButtonText->SetText(FText::FromString(TEXT("확정 요청 중...")));
}

void UOSCharSelectWidget::OnCancelClicked()
{
	// ── 서버에 확정 취소 요청 ──
	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;
	AOSPlayerState* PS = Cast<AOSPlayerState>(PC->PlayerState);
	if (!PS) return;
	PS->Server_RequestCancelConfirm();

	UpdateButtonStates();
}

// ═══════════════════════════════════════════
//  버튼 상태 관리
// ═══════════════════════════════════════════
void UOSCharSelectWidget::UpdateButtonStates()
{
	bool bConfirmed = false;

	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
	    AOSPlayerState* PS = Cast<AOSPlayerState>(PC->PlayerState);
	    bConfirmed = PS ? PS->IsCharacterConfirmed() : false;
	}

	if (bConfirmed)
	{
		if (ConfirmButton) ConfirmButton->SetIsEnabled(false);
		if (ConfirmButtonText) ConfirmButtonText->SetText(FText::FromString(TEXT("확정")));
		if (CancelButton) CancelButton->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		if (ConfirmButton) ConfirmButton->SetIsEnabled(!SelectedID.IsNone());
		if (ConfirmButtonText) ConfirmButtonText->SetText(FText::FromString(TEXT("선택")));
		if (CancelButton) CancelButton->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UOSCharSelectWidget::OnMyConfirmChanged(AOSPlayerState* Player, bool bConfirmed)
{
	LOG_GT_W(TEXT("★★ Widget: OnMyConfirmChanged bConfirmed=%d"), bConfirmed);
	RefreshCardStates();
	UpdateButtonStates();
}

void UOSCharSelectWidget::TryBindPlayerState()
{
	APlayerController* pc = GetOwningPlayer();
	if (!pc) return;

	AOSPlayerState* ps = Cast<AOSPlayerState>(pc->PlayerState);
	if (!ps)
	{
		// PlayerState 아직 없음 → 0.1초 후 재시도
		GetWorld()->GetTimerManager().SetTimer(
			BindTimerHandle, this, 
			&UOSCharSelectWidget::TryBindPlayerState, 0.1f, false);
		return;
	}

	// 바인딩 성공
	ps->OnSelectRejected.AddDynamic(this, &UOSCharSelectWidget::OnMySelectRejected);
	ps->OnPlayerConfirmChanged.AddDynamic(this, &UOSCharSelectWidget::OnMyConfirmChanged);
    
	// 혹시 이미 확정된 상태면 즉시 반영
	UpdateButtonStates();
	RefreshCardStates();
    
	LOG_GT_W(TEXT("★ PlayerState 바인딩 완료: %s"), *ps->GetPlayerName());
}

// ═══════════════════════════════════════════
//  ★ 게임 시작 버튼 (호스트 전용)
// ═══════════════════════════════════════════
void UOSCharSelectWidget::OnStartGameClicked()
{
	AOSCharSelectGameMode* GM = Cast<AOSCharSelectGameMode>(GetWorld()->GetAuthGameMode());
	if (GM)
	{
		GM->StartArenaTravel();
	}
}

void UOSCharSelectWidget::UpdateStartGameButton()
{
	if (!StartGameButton) return;
	
	// 호스트가 아니면 숨김 유지
	if (GetWorld()->GetAuthGameMode() == nullptr) return;
	
	bool bAllConfirmed = CachedGameState ? CachedGameState->AreAllPlayersConfirmed() : false;
	StartGameButton->SetIsEnabled(bAllConfirmed);
	
	if (StartGameButtonText)
	{
		StartGameButtonText->SetText(
			bAllConfirmed
			? FText::FromString(TEXT("게임 시작"))
			: FText::FromString(TEXT("전원 확정 대기 중..."))
		);
	}
}