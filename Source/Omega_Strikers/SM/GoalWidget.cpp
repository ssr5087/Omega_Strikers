// Fill out your copyright notice in the Description page of Project Settings.


#include "GoalWidget.h"

#include "Components/Image.h"

void UGoalWidget::PlayGoalAnimation(int32 ScorerIndex)
{
	switch (ScorerIndex)
	{
		case 0:
			Image_Luna->SetVisibility(ESlateVisibility::Visible);
			break;
		case 1:
			Image_Asher->SetVisibility(ESlateVisibility::Visible);
			break;
		case 2:
			Image_Aimi->SetVisibility(ESlateVisibility::Visible);
			break;
		default:
			break;
	}
	
	PlayAnimation(Goal);
}
