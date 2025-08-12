// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UnitAIInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UUnitAIInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class TOPDOWN_UTILITIES_API IUnitAIInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// 它应该返回该AI应该默认运行的行为树。
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AI")
    UBehaviorTree* GetDefaultBehaviorTree() const;
};
