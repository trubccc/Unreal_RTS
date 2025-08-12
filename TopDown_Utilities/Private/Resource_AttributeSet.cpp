// Fill out your copyright notice in the Description page of Project Settings.


#include "Resource_AttributeSet.h"
#include "Net/UnrealNetwork.h"

void UResource_AttributeSet::OnRep_Wood(const FGameplayAttributeData& OldWood)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UResource_AttributeSet, Wood, OldWood);
}

void UResource_AttributeSet::OnRep_Food(const FGameplayAttributeData& OldFood)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UResource_AttributeSet, Food, OldFood);
}

void UResource_AttributeSet::OnRep_Stone(const FGameplayAttributeData& OldStone)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UResource_AttributeSet, Stone, OldStone);
}

void UResource_AttributeSet::OnRep_Gold(const FGameplayAttributeData& OldGold)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UResource_AttributeSet, Gold, OldGold);
}

void UResource_AttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME_CONDITION_NOTIFY(UResource_AttributeSet, Wood, COND_None, REPNOTIFY_Always);
}
