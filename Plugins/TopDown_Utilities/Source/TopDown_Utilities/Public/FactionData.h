// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FactionData.generated.h"
/**
 * 
 */
USTRUCT(BlueprintType)
struct FFactionData : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
	FLinearColor FactionColor;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
	FString FactionName;
	
	FFactionData() : FactionColor(FLinearColor::Blue), FactionName(TEXT("Rebels")){}

};
