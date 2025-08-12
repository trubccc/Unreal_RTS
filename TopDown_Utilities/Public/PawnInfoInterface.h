// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PawnInfoInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPawnInfoInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class TOPDOWN_UTILITIES_API IPawnInfoInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn Info Interface")
	void GetHealthAttributes(float& OutCurrentHealth, float& OutMaxHealth);

	//UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn Info Interface")
	//void SetHealthBarWidget(UUserWidget* HealthBarWidget);
};
