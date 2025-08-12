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

USTRUCT(BlueprintType)
struct FHealthUpdateMessage
{
	GENERATED_BODY()

	// 发送者，也就是哪个Pawn的血量变化了
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gameplay Message")
	AActor* Sender;

	// 当前血量
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gameplay Message")
	float CurrentHealth;

	// 最大血量
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gameplay Message")
	float MaxHealth;
};