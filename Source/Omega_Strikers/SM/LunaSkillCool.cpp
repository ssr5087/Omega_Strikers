// Fill out your copyright notice in the Description page of Project Settings.


#include "LunaSkillCool.h"

void ULunaSkillCool::LoadCore()
{
	PlayAnimation(CoreCool);
}

void ULunaSkillCool::LoadPrim()
{
	PlayAnimation(PrimCool);
}

void ULunaSkillCool::LoadSeco()
{
	PlayAnimation(SecoCool);
}

void ULunaSkillCool::LoadSpec()
{
	PlayAnimation(SpecCool);
}
