// Fill out your copyright notice in the Description page of Project Settings.


#include "SessionMode.h"

#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Omega_Strikers/Omega_Strikers.h"

ASessionMode::ASessionMode()
{
	// 심리스 트래블 지원
	bUseSeamlessTravel = true;
}