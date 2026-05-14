// Fill out your copyright notice in the Description page of Project Settings.


#include "HPStatusWidget.h"

#include "OSType.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UHPStatusWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
}

void UHPStatusWidget::SetTeamSide(bool bIsMyPlayer, EOSTeam teamSide)
{
	if (!img_StaggerBar_Teamside) {return;}
	if (bIsMyPlayer)
	{
		img_StaggerBar_Teamside->SetBrushFromMaterial(MyPlayer);
		CurrentStaggerBar = img_StaggerBar_Teamside->GetDynamicMaterial();
		return;
	}
	
	if (teamSide == EOSTeam::Blue)
	{
		img_StaggerBar_Teamside->SetBrushFromMaterial(BlueTeam);
	}
	else
	{
		img_StaggerBar_Teamside->SetBrushFromMaterial(RedTeam);
	}
	CurrentStaggerBar = img_StaggerBar_Teamside->GetDynamicMaterial();
}

void UHPStatusWidget::SetStaggerPercent(float Percent)
{
	CurrentStaggerBar = img_StaggerBar_Teamside->GetDynamicMaterial();
	
	if (CurrentStaggerBar)
	{
		CurrentStaggerBar->SetScalarParameterValue("Gauge", Percent);
	}
}

void UHPStatusWidget::SetLevel(int32 level)
{
	txt_level->SetText(FText::AsNumber(level));
}
