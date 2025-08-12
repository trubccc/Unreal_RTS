// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "CommonENUMS.h"
#include "ResourceInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UResourceInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class TOPDOWN_UTILITIES_API IResourceInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
    // 存放资源
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void DepositResource(EResourceType Type, int32 Amount);

    // 获取采集这个资源所需要的能力标签 (比如 "Ability.ChopTree")
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    FGameplayTag GetRequiredAbilityTag() const;
};
