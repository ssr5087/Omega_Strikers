// Fill out your copyright notice in the Description page of Project Settings.


#include "ResultWidget.h"

#include "Components/TextBlock.h"

void UResultWidget::SetScore(int32 blue, int32 red)
{
	txt_BlueScore->SetText(FText::AsNumber(blue));
	txt_RedScore->SetText(FText::AsNumber(red));
	
	txt_Score_B2->SetText(FText::AsNumber(3));
	txt_Score_R2->SetText(FText::AsNumber(3));
	
	switch (blue)
	{
		case 4:
			txt_Score_B3->SetText(FText::AsNumber(1));
			break;
		case 5:
			txt_Score_B1->SetText(FText::AsNumber(1));
			txt_Score_B3->SetText(FText::AsNumber(1));
			break;
		default:
			txt_Score_B2->SetText(FText::AsNumber(blue));
			break;
	}
	
	switch (red)
	{
		case 4:
			txt_Score_R3->SetText(FText::AsNumber(1));
			break;
		case 5:
			txt_Score_R1->SetText(FText::AsNumber(1));
			txt_Score_R3->SetText(FText::AsNumber(1));
			break;
		default:
			txt_Score_R2->SetText(FText::AsNumber(red));
			break;
	}
}
