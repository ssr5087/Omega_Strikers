// Fill out your copyright notice in the Description page of Project Settings.


#include "HPComponent.h"


// Sets default values for this component's properties
UHPComponent::UHPComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UHPComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UHPComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	GEngine->AddOnScreenDebugMessage(1, DeltaTime, FColor::Red, FString::Printf(TEXT("현재 체력 : %.0f / %.0f"), CurHP, MaxHP));

	// ...
}

void UHPComponent::InitializeHP()
{
	CurHP = MaxHP;
}

void UHPComponent::UpdateMaxHP(float NewMax)
{
	// 추후 레벨업을 별도 컴포넌트에서 관리하게 된다면, 캐릭터가 그 값을 전달받고 맥스 값만 HP컴포넌트에게 전달
	// HP컴포넌트는 이 함수로 업데이트만 하면 됨
	MaxHP = NewMax;
	// 레벨 업 시에 현재 체력은 갱신 안 되니까 dotheal함수 타이머 호출 추가할 필요 있음
}

void UHPComponent::ApplyDamage(float DamageAmount)
{
	// 힐이 진행 중이었다면 타이머 멈추기
	GetWorld()->GetTimerManager().ClearTimer(HealTimer);
	
	// 현재 체력에 데미지 반영하고 clamp 
	CurHP = FMath::Clamp(CurHP - DamageAmount, 0.0f, MaxHP);
	
	// 체력이 처음으로 0이 되는 순간(경직이 진행되지 않은 상태에서 체력이 0이 된 순간)
	if (CurHP <= 0 && !bIsStaggered)
	{
		// 이제 경직이 활성화 되었으므로 bool값을 바꿔주고, 델리게이트로 캐릭터에게 알림
		bIsStaggered = true;
		OnHPBecomeNegative.ExecuteIfBound();
	}
	
	// 데미지 감소 적용 후, 3초 후부터 일정 시간 간격으로 지속적으로 회복됨(회복 함수 호출)
	// 체력이 0에서 최대까지 차는데 25초 정도 걸리므로 1초에 4% 정도 회복됨 -> 0.2초에 1% 회복으로 구현
	GetWorld()->GetTimerManager().SetTimer(HealTimer, this, &UHPComponent::DotHeal, 0.2f, true, 3.0f);
}

void UHPComponent::DotHeal()
{
	// 힐 타이머가 돌아가는 동안은 0.2초마다 최대 체력의 1% 만큼씩 회복됨
	CurHP = FMath::Clamp(CurHP + MaxHP * 0.01f, 0.0f, MaxHP);
	
	// 체력이 처음으로 50% 이상 올라가는 순간(경직 상태에서 체력이 양수가 된 순간)
	if (CurHP >= MaxHP / 2 && bIsStaggered)
	{
		// 이제 경직이 풀리므로 bool값을 바꿔주고, 델리게이트로 캐릭터에게 알림
		bIsStaggered = false;
		OnHPBecomePositive.ExecuteIfBound();
	}
	
	// 체력이 모두 회복되었으면 힐 타이머 종료
	if (CurHP >= MaxHP)
	{
		GetWorld()->GetTimerManager().ClearTimer(HealTimer);
	}
}

void UHPComponent::ApplyHeal(float HealAmount)
{
	// 얘도 힐 적용 후 clamp
	CurHP = FMath::Clamp(CurHP + HealAmount, 0.0f, MaxHP);
	
	// 체력이 처음으로 0 초과로 올라가는 순간(경직 상태에서 체력이 양수가 된 순간)
	if (CurHP > 0 && bIsStaggered)
	{
		// 이제 경직이 풀리므로 bool값을 바꿔주고, 델리게이트로 캐릭터에게 알림
		bIsStaggered = false;
		OnHPBecomePositive.ExecuteIfBound();
	}
}
