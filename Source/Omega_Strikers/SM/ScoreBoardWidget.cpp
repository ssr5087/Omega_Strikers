// Fill out your copyright notice in the Description page of Project Settings.


#include "ScoreBoardWidget.h"
#include "Components/TextBlock.h"

void UScoreBoardWidget::SetScore(int32 Blue, int32 Red)
{
	txt_ScoreBlue->SetText(FText::AsNumber(Blue));
	txt_ScoreRed->SetText(FText::AsNumber(Red));
}
