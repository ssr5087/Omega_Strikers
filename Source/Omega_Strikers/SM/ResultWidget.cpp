// Fill out your copyright notice in the Description page of Project Settings.


#include "ResultWidget.h"

#include "Components/TextBlock.h"

void UResultWidget::SetScore(int32 blue, int32 red, int32 B1, int32 B2, int32 B3, int32 R1,int32 R2, int32 R3)
{
	txt_BlueScore->SetText(FText::AsNumber(blue));
	txt_RedScore->SetText(FText::AsNumber(red));
	
	txt_Score_B1->SetText(FText::AsNumber(B1));
	txt_Score_R1->SetText(FText::AsNumber(R1));
	txt_Score_B2->SetText(FText::AsNumber(B2));
	txt_Score_R2->SetText(FText::AsNumber(R2));
	txt_Score_B3->SetText(FText::AsNumber(B3));
	txt_Score_R3->SetText(FText::AsNumber(R3));
}
