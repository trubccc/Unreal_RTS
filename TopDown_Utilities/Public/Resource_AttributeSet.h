// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Resource_AttributeSet.generated.h"


#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
		GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
		GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
		GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
		GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)


/**
 * 
 */
UCLASS()
class TOPDOWN_UTILITIES_API UResource_AttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "Resources", ReplicatedUsing = OnRep_Wood)
	FGameplayAttributeData Wood;
	ATTRIBUTE_ACCESSORS(UResource_AttributeSet, Wood);

	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "Resources", ReplicatedUsing = OnRep_Food)
	FGameplayAttributeData Food;
	ATTRIBUTE_ACCESSORS(UResource_AttributeSet, Food);

	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "Resources", ReplicatedUsing = OnRep_Stone)
	FGameplayAttributeData Stone;
	ATTRIBUTE_ACCESSORS(UResource_AttributeSet, Stone);

	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "Resources", ReplicatedUsing = OnRep_Gold)
	FGameplayAttributeData Gold;
	ATTRIBUTE_ACCESSORS(UResource_AttributeSet, Gold);
	
	UFUNCTION()
	void OnRep_Wood(const FGameplayAttributeData& OldWood);

	UFUNCTION()
	void OnRep_Food(const FGameplayAttributeData& OldFood);

	UFUNCTION()
	void OnRep_Stone(const FGameplayAttributeData& OldStone);

	UFUNCTION()
	void OnRep_Gold(const FGameplayAttributeData& OldGold);


	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
