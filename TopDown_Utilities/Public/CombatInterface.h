// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CombatInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UCombatInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class TOPDOWN_UTILITIES_API ICombatInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
    
// 函数：命令此单位去攻击一个目标
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	void CommandAttackTarget(AActor* TargetToAttack);

    // 函数：此单位受到伤害
    // DamageAmount: 受到的伤害数值
    // DamageCauser: 造成伤害的来源Actor
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	void TakeDamage(float DamageAmount, AActor* DamageCauser);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	void PlayerCommandAttackTarget(AActor* TargetToAttack);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	bool IsDead();
};
