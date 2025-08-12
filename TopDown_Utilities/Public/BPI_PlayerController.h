// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BPI_PlayerController.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UBPI_PlayerController : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class TOPDOWN_UTILITIES_API IBPI_PlayerController
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	// 声明一个可以在C++中实现并在蓝图中调用的事件
	// BlueprintNativeEvent 意味着它可以在C++中有一个默认实现，也可以在蓝图中被重写
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Hover Interface")
	void OnHover();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Hover Interface")
	void ClearHover();
};
