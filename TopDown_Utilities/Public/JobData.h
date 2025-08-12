// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "JobData.generated.h"

/**
 * 
 */
class UBehaviorTree;
class UAnimMontage;

USTRUCT(BlueprintType)
struct FJobData : public FTableRowBase
{
	GENERATED_BODY()

	// 这个职业对应的行为树
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UBehaviorTree> BehaviorTree;

	// 这个职业执行时播放的动画
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UAnimMontage> WorkMontage;

	// 这个职业使用的工具 (可选)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UStaticMesh> ToolMesh;
};
