// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_GatherResource.generated.h"

/**
 * 
 */
UCLASS()
class TOPDOWN_UTILITIES_API UGA_GatherResource : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
    UGA_GatherResource();
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
    // 目标资源
    UPROPERTY()
	TObjectPtr<AActor> TargetResource;

    // 播放采集动画的Task
    UFUNCTION()
    void OnAnimationFinished();	

	UPROPERTY(EditDefaultsOnly, Category="Animation")
	TObjectPtr<UAnimMontage> GatherMontage;


};
