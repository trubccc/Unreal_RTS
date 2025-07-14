// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonENUMS.h"
#include "AttributeSet.h"
#include "ActorData.generated.h"
/**
 * 
 */

USTRUCT(BlueprintType)
struct FActorData : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	EActorType ActorType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	TObjectPtr<UTexture2D> Texture;	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	TObjectPtr<UMaterial> IconMaterial;	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	int32 BaseHealth;	
	
	FActorData() : ActorType(EActorType::Villager), DisplayName(TEXT("")), Texture(nullptr), BaseHealth(100) {}


	FActorData(EActorType Type, FString Name, TObjectPtr<UTexture2D> Tex, int32 Default)
		: ActorType(Type), DisplayName(Name), Texture(Tex), BaseHealth(Default)
	{
	}
};

USTRUCT(BlueprintType)
struct FBuildCastData : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	EActorType ActorType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	TMap<FGameplayAttribute, int32> BuildCast;


};

