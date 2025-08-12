// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h" 
#include "ResourceData.h"
#include "AbilitySystemComponent.h"
#include "TopDown_PlayerState.generated.h"

class UResource_AttributeSet;
class UAbilitySystemComponent;
struct FOnAttributeChangeData;
/**
 * 
 */
UCLASS()
class TOPDOWN_UTILITIES_API ATopDown_PlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
private:

	//
	void OnResourceHarvested(FGameplayTag Channel, const FResourceHarvestedMessage& Message);
	// 资源变量
	int32 WoodCount = 0;
	int32 StoneCount = 0;

public:
	ATopDown_PlayerState();

	virtual void BeginPlay() override;
	
	UFUNCTION(BlueprintCallable, Category = "Resources")
	bool CanBuildCast(const TMap<FGameplayAttribute, int32>& BuildCast);

	UFUNCTION(BlueprintCallable, Category = "Resources")
	void SpendBuildCost(const TMap<FGameplayAttribute, int32>& BuildCost);

	UFUNCTION(BlueprintCallable, Category = "Resources")
	void RevertBuildCast(const TMap<FGameplayAttribute, int32>& BuildCast);
	

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;


protected:


	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "Resources")
	const class UResource_AttributeSet* ResourceAttributeSet;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "Resources")
	UAbilitySystemComponent* AbilitySystemComponent;
	
	virtual void WoodCountChanged(const FOnAttributeChangeData& ChangeData);
	virtual void StoneCountChanged(const FOnAttributeChangeData& ChangeData);
	virtual void GoldCountChanged(const FOnAttributeChangeData& ChangeData);
	virtual void FoodCountChanged(const FOnAttributeChangeData& ChangeData);
	

	UFUNCTION(BlueprintCallable, Category = "Resources")
	void OnResourcesCountChanged();
};
