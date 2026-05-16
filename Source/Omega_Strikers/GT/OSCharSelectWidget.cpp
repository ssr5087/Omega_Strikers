// Fill out your copyright notice in the Description page of Project Settings.
// 천전천승 — 캐릭터 선택 화면 구현

#include "OSCharSelectWidget.h"

#include "OSCharCardWidget.h"
#include "OSCharSelectGameMode.h"
#include "OSCharSelectGameState.h"
#include "OSGameInstance.h"
#include "OSPlayerState.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Omega_Strikers/Omega_Strikers.h"
#include "Omega_Strikers/SSR/CharacterStat.h"

void UOSCharSelectWidget::NativeConstruct()
{
	Super::NativeConstruct();

	GetWorld()->GetTimerManager().ClearTimer(BindTimerHandle);

	// 시작부터 텍스쳐 프리뷰 숨기기
	if (PreviewImage)
	{
		PreviewImage->SetVisibility(ESlateVisibility::Hidden);
	}
	
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
	
	// 게임 시작 버튼 — 호스트(서버)에게만 표시
	if (StartGameButton)
	{
		StartGameButton->OnClicked.AddDynamic(this, &UOSCharSelectWidget::OnStartGameClicked);
		
		bool bIsHost = (GetWorld()->GetAuthGameMode() != nullptr);
		StartGameButton->SetVisibility(bIsHost ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		StartGameButton->SetIsEnabled(false);
	}
	
	// ══════════════════════════════════════
	// 팀 선택 버튼 바인딩
	// ══════════════════════════════════════
	if (TeamBlueButton)
	{
		TeamBlueButton->OnClicked.AddDynamic(this, &UOSCharSelectWidget::OnTeamBlueClicked);
	}
	if (TeamRedButton)
	{
		TeamRedButton->OnClicked.AddDynamic(this, &UOSCharSelectWidget::OnTeamRedClicked);
	}
	
	BuildGrid();
	
	// GameState 구독
	CachedGameState = GetWorld()->GetGameState<AOSCharSelectGameState>();
	if ( CachedGameState )
	{
		CachedGameState->OnCharSelectListUpdated.AddDynamic(this, &UOSCharSelectWidget::OnCharSelectListUpdated);
	}
	
	// GameState + PlayerState 모두 타이머로 바인딩
	TryBindAll();
	
	// 초기 UI 반영
	RefreshCardStates();
	UpdateButtonStates();
	UpdateStartGameButton();
	UpdateTeamUI(); // 팀 UI 초기화
	
	// 0.5초마다 팀 UI 갱신 (타 플레이어 팀 변경 반영)
	GetWorld()->GetTimerManager().SetTimer(TeamUITimerHandle, this, &UOSCharSelectWidget::UpdateTeamUI, 0.5f, true);
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
		
		// 초상화 자동 로드
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
	// 전신 이미지 자동 로드
	/*if (PreviewImage != nullptr)
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
	}*/

	// ── 기존 텍스쳐 프리뷰 숨기기 ──
	if (PreviewImage)
	{
		PreviewImage->SetVisibility(ESlateVisibility::Hidden);
	}

	// ── 기존 프리뷰 캐릭터 제거 ──
	if (PreviewCharacter)
	{
		PreviewCharacter->Destroy();
		PreviewCharacter = nullptr;
	}

	// ── 새 캐릭터 스폰 ──
	TSubclassOf<ACharacter>* BPClass = CharacterBPMap.Find(CharacterID);
	if (BPClass && *BPClass)
	{
		// PlayerStart 위치 찾기
		AActor* StartSpot = UGameplayStatics::GetActorOfClass(
			GetWorld(), APlayerStart::StaticClass());

		if (StartSpot)
		{
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			PreviewCharacter = GetWorld()->SpawnActor<ACharacter>(*BPClass, StartSpot->GetActorTransform(), Params);

			if (PreviewCharacter)
			{
				// 스폰 직후에 리플리케이션 끄기
				PreviewCharacter->SetReplicates(false);
				
				// 프리뷰용이므로 이동/AI 비활성화
				PreviewCharacter->DisableInput(nullptr);
				if (auto* Movement = PreviewCharacter->GetCharacterMovement())
				{
					Movement->GravityScale = 0.f;
					Movement->StopMovementImmediately();
				}
			}
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
	int32 myTeam = ps->GetTeamID();
	if (CachedGameState && CachedGameState->IsCharacterLocked(CharacterID, myTeam)) return;

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
	// 세션에서 나가기 (호스트면 세션 파괴 포함)
	UOSGameInstance* gi = Cast<UOSGameInstance>(GetGameInstance());
	if ( gi )
	{
		gi->LeaveSession();
	}
	
	RemoveFromParent();
}

void UOSCharSelectWidget::NativeDestruct()
{
	GetWorld()->GetTimerManager().ClearTimer(BindTimerHandle);
	
	if ( CachedGameState )
	{
		CachedGameState->OnCharSelectListUpdated.RemoveDynamic(this, &UOSCharSelectWidget::OnCharSelectListUpdated);
	}
	
	// 팀 관련 델리게이트 정리
	if (APlayerController* pc = GetOwningPlayer())
	{
		AOSPlayerState* ps = Cast<AOSPlayerState>(pc->PlayerState);
		if (ps)
		{
			ps->OnTeamAssigned.RemoveDynamic(this, &UOSCharSelectWidget::OnAnyTeamChanged);
			ps->OnTeamChangeRejected.RemoveDynamic(this, &UOSCharSelectWidget::OnMyTeamChangeRejected);
		}
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
	UpdateTeamUI();
}

void UOSCharSelectWidget::RefreshCardStates()
{
	if ( !CachedGameState ) return;
	
	APlayerController* pc = GetOwningPlayer();
	if ( !pc ) return;
	
	int32 myPID = -1;
	int32 myTeam = -1;
	if (AOSPlayerState* ps = Cast<AOSPlayerState>(pc->PlayerState))
	{
		myPID = ps->GetPlayerId();
		myTeam = ps->GetTeamID();
	}
	
	for (UOSCharCardWidget* card : CardWidgets)
	{
		if ( !card ) continue;
		
		FName cid = card->GetCharacterID();
		
		bool bLockedByOther = false;
		bool bLockedByMe = false;
		bool bSelectedByMe = (cid == SelectedID);
		
		// 팀별 확정자 이름 수집 (break 안 함!)
		FString sameTeamLockerName;   // 같은 팀에서 확정한 사람 이름
		FString otherTeamLockerName;  // 다른 팀에서 확정한 사람 이름
		        
		for (const FOSCharSelectEntry& entry : CachedGameState->CharSelectList)
		{
			if (entry.CharacterID == cid && entry.bConfirmed)
			{
				if (entry.CharacterID != cid || !entry.bConfirmed) continue;
				if (entry.PlayerIndex == myPID) bLockedByMe = true;
				else if (entry.TeamID == myTeam) // 같은 팀일 경우만 중복 벤
				{
					// 같은 팀 다른 플레이어 -> 잠금 (선택 불가)
					bLockedByOther = true;
					sameTeamLockerName = entry.PlayerName;
				}
				else
				{
					// 다른 팀 플레이어 -> 잠금 안 함, 이름만 수집
					otherTeamLockerName = entry.PlayerName;
				}
			}
		}
		
		// 카드 비주얼 업데이트
		if ( bLockedByOther )
		{
			// 같은 팀 다른 플레이어가 확정 → 회색 잠금 + 클릭 불가
			card->SetSelected(false);
			card->SetLocked(true, sameTeamLockerName, otherTeamLockerName);
		}
		else if ( bLockedByMe )
		{
			// 내가 확정 — 하이라이트 + 다른 팀 이름 표시
			card->SetSelected(true);
			card->SetLocked(false, FString(), otherTeamLockerName);
		}
		else if ( bSelectedByMe )
		{
			// 아직 확정 전, 내가 선택 중
			card->SetSelected(true);
			card->SetLocked(false, FString(), otherTeamLockerName);
		}
		else
		{
			// 아무도 안 골랐거나, 다른 팀만 골랐을 때
			card->SetSelected(false);
			card->SetLocked(false, FString(), otherTeamLockerName);
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
	
	// 팀 미배정이면 확정 불가
	if (PS->GetTeamID() == -1)
	{
		if (StatusText)
		{
			StatusText->SetText(FText::FromString(TEXT("먼저 팀을 선택해주세요.")));
			StatusText->SetColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.2f, 0.2f, 1.f)));
			StatusText->SetVisibility(ESlateVisibility::Visible);
		}
		return;
	}

	
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
		
		// 확정 후 팀 교체 불가
		if (TeamBlueButton) TeamBlueButton->SetIsEnabled(false);
		if (TeamRedButton) TeamRedButton->SetIsEnabled(false);
	}
	else
	{
		if (ConfirmButton) ConfirmButton->SetIsEnabled(!SelectedID.IsNone());
		if (ConfirmButtonText) ConfirmButtonText->SetText(FText::FromString(TEXT("선택")));
		if (CancelButton) CancelButton->SetVisibility(ESlateVisibility::Collapsed);
		
		// 확정 취소 시 팀 버튼 재활성화
		UpdateTeamUI();
	}
}

void UOSCharSelectWidget::TryBindAll()
{
	bool bNeedRetry = false;
    
	// ── GameState 바인딩 ──
	if (!CachedGameState)
	{
		CachedGameState = GetWorld()->GetGameState<AOSCharSelectGameState>();
		if (CachedGameState)
		{
			CachedGameState->OnCharSelectListUpdated.AddDynamic(
				this, &UOSCharSelectWidget::OnCharSelectListUpdated);
			LOG_GT_W(TEXT("★ GameState 바인딩 완료"));
		}
		else
		{
			bNeedRetry = true;
		}
	}
    
	// ── PlayerState 바인딩 ──
	if (!bPlayerStateBound)
	{
		if (APlayerController* pc = GetOwningPlayer())
		{
			AOSPlayerState* ps = Cast<AOSPlayerState>(pc->PlayerState);
			if (ps)
			{
				ps->OnSelectRejected.AddDynamic(this, &UOSCharSelectWidget::OnMySelectRejected);
				// 팀 관련 델리게이트 바인딩
				ps->OnTeamAssigned.AddDynamic(this, &UOSCharSelectWidget::OnAnyTeamChanged);
				ps->OnTeamChangeRejected.AddDynamic(this, &UOSCharSelectWidget::OnMyTeamChangeRejected);
				
				bPlayerStateBound = true;
				
				LOG_GT_W(TEXT("★ PlayerState 바인딩 완료: %s"), *ps->GetPlayerName());
			}
			else
			{
				bNeedRetry = true;
			}
		}
	}
    
	// 둘 다 완료되면 초기 UI 갱신
	if (CachedGameState && bPlayerStateBound)
	{
		RefreshCardStates();
		UpdateButtonStates();
		UpdateStartGameButton();
		UpdateTeamUI();
		return;
	}
    
	// 아직 미완 → 0.1초 후 재시도
	if (bNeedRetry)
	{
		GetWorld()->GetTimerManager().SetTimer(
			BindTimerHandle, this,
			&UOSCharSelectWidget::TryBindAll, 0.1f, false);
	}
}

// ═══════════════════════════════════════════
//  게임 시작 버튼 (호스트 전용)
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

// ═══════════════════════════════════════════════════════
// 팀 선택
// ═══════════════════════════════════════════════════════
void UOSCharSelectWidget::OnTeamBlueClicked()
{
	RequestTeamChange(0);
}

void UOSCharSelectWidget::OnTeamRedClicked()
{
	RequestTeamChange(1);
}

void UOSCharSelectWidget::RequestTeamChange(int32 NewTeamID)
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	AOSPlayerState* PS = Cast<AOSPlayerState>(PC->PlayerState);
	if (!PS) return;

	// 이미 같은 팀이면 무시
	if (PS->GetTeamID() == NewTeamID) return;

	// 확정 상태면 팀 변경 불가
	if (PS->IsCharacterConfirmed()) return;

	// 서버에 팀 변경 요청
	PS->Server_RequestTeamChange(NewTeamID);
}

void UOSCharSelectWidget::UpdateTeamUI()
{
	// 모든 PlayerState 순회 → 팀별 인원 수
	int32 teamBlueCount = 0;
	int32 teamRedCount = 0;
	int32 maxPerTeam = 3;

	UWorld* world = GetWorld();
	if (!world) return;

	// GameMode에서 MaxPlayersPerTeam 가져오기 (서버만)
	AOSCharSelectGameMode* gm = Cast<AOSCharSelectGameMode>(world->GetAuthGameMode());
	if (gm)
	{
		// 서버에서만 접근 가능
		// MaxPerTeam은 PostLogin에서 세션 기반으로 자동 계산됨
	}

	// RequiredPlayerCount 기반으로 계산 (클라이언트도 가능)
	if (CachedGameState && CachedGameState->RequiredPlayerCount > 0)
	{
		maxPerTeam = FMath::CeilToInt(CachedGameState->RequiredPlayerCount / 2.0f);
	}

	AGameStateBase* gs = world->GetGameState<AGameStateBase>();
	if (gs)
	{
		for (APlayerState* BasePS : gs->PlayerArray)
		{
			AOSPlayerState* ps = Cast<AOSPlayerState>(BasePS);
			if (!ps) continue;

			if (ps->GetTeamID() == 0) teamBlueCount++;
			else if (ps->GetTeamID() == 1) teamRedCount++;
		}
	}

	// ─── 팀 인원 텍스트 갱신 ───
	if (TeamBlueText)
	{
		TeamBlueText->SetText(FText::FromString(
			FString::Printf(TEXT("블루팀 (%d/%d)"), teamBlueCount, maxPerTeam)));
	}
	if (TeamRedText)
	{
		TeamRedText->SetText(FText::FromString(
			FString::Printf(TEXT("레드팀 (%d/%d)"), teamRedCount, maxPerTeam)));
	}

	// ─── 내 팀 표시 ───
	APlayerController* pc = GetOwningPlayer();
	AOSPlayerState* myPs = pc ? Cast<AOSPlayerState>(pc->PlayerState) : nullptr;
	int32 MyTeam = myPs ? myPs->GetTeamID() : -1;

	if (MyTeamText)
	{
		if (MyTeam == -1)
		{
			MyTeamText->SetText(FText::FromString(TEXT("미정")));
			MyTeamText->SetColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f, 1.f)));
		}
		else
		{
			FString TeamName = (MyTeam == 0) ? TEXT("블루팀") : TEXT("레드팀");
			MyTeamText->SetText(FText::FromString(
				FString::Printf(TEXT("현재 팀: %s"), *TeamName)));

			FLinearColor TeamColor = (MyTeam == 0)
				? FLinearColor(0.3f, 0.5f, 0.9f, 1.f)
				: FLinearColor(0.9f, 0.3f, 0.3f, 1.f);  
			MyTeamText->SetColorAndOpacity(FSlateColor(TeamColor));
		}
	}

	// ─── 팀 버튼 상태 ───
	// 내가 속한 팀 버튼은 비활성 (이미 그 팀)
	// 확정 상태면 둘 다 비활성
	bool bConfirmed = myPs ? myPs->IsCharacterConfirmed() : false;

	if (TeamBlueButton)
	{
		TeamBlueButton->SetIsEnabled(!bConfirmed && MyTeam != 0);
	}
	if (TeamRedButton)
	{
		TeamRedButton->SetIsEnabled(!bConfirmed && MyTeam != 1);
	}
}

void UOSCharSelectWidget::OnAnyTeamChanged(AOSPlayerState* Player, int32 NewTeamID)
{
	// 내 팀이 변경됐을 때 UI 즉시 갱신
	UpdateTeamUI();

	// 상태 메시지
	APlayerController* PC = GetOwningPlayer();
	if (PC && Player == Cast<AOSPlayerState>(PC->PlayerState))
	{
		if (StatusText)
		{
			FString TeamName = (NewTeamID == 0) ? TEXT("A") : TEXT("B");
			StatusText->SetText(FText::FromString(
				FString::Printf(TEXT("%s팀에 참가했습니다."), *TeamName)));
			StatusText->SetColorAndOpacity(FSlateColor(
				(NewTeamID == 0)
					? FLinearColor(0.3f, 0.5f, 0.9f, 1.f)
					: FLinearColor(0.9f, 0.3f, 0.3f, 1.f)));
			StatusText->SetVisibility(ESlateVisibility::Visible);
		}
	}
}

void UOSCharSelectWidget::OnMyTeamChangeRejected(const FString& Reason)
{
	if (StatusText)
	{
		StatusText->SetText(FText::FromString(Reason));
		StatusText->SetColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.2f, 0.2f, 1.f)));
		StatusText->SetVisibility(ESlateVisibility::Visible);
	}
}
