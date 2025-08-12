// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonENUMS.h"
#include "GameplayTagContainer.h"
#include "AttributeSet.h"
#include "Engine/DataTable.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "ResourceData.generated.h"
/**
 * 
 */
USTRUCT(BlueprintType)
struct FResourceData : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	EResourceType ResourceType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	TObjectPtr<UTexture2D> Texture;	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	int32 DefaultValue;	
	
	FResourceData() : ResourceType(EResourceType::Wood), DisplayName(TEXT("")), Texture(nullptr), DefaultValue(0) {}


	FResourceData(EResourceType Type, FString Name, TObjectPtr<UTexture2D> Tex, int32 Default) : ResourceType(Type), DisplayName(Name), Texture(Tex), DefaultValue(Default)
	{
	}
};

USTRUCT(BlueprintType)
struct FResourceMessage

{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	TArray<FGameplayAttribute> ResourcesAttribute;

};



USTRUCT(BlueprintType)
struct FResourceHarvestedMessage
{
    //
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TObjectPtr<AActor> Harvester;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TObjectPtr<AActor> Resource;
	// --- 补全结束 ---

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 Amount = 0;
    
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FGameplayTag ResourceType;
};