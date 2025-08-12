// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "BPI_Villager.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UBPI_Villager : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class TOPDOWN_UTILITIES_API IBPI_Villager
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ChangeJob(const FGameplayTag& NewJobTag, AActor* JobTarget);

	// 播放工作动画
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void PlayWorkAnimation();

	// 停止工作动画
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void StopWorkAnimation();
};
