// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayMessageStructures.generated.h"
/**
 * 
 */
USTRUCT(BlueprintType)
struct FCommonGamePlayMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnyWhere, Category = "Common Gameplay Message")
	FString MessageText;

	UPROPERTY(BlueprintReadWrite, EditAnyWhere, Category = "Common Gameplay Message")
	AActor* Sender;
	
};