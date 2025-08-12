// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "TopDown_GameState.generated.h"

/**
 * 
 */
UCLASS()
class TOPDOWN_UTILITIES_API ATopDown_GameState : public AGameStateBase
{
	GENERATED_BODY()

protected:
    // 只有服务器需要填充和复制这个Map
    virtual void BeginPlay() override;



    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// 一个公开的函数，允许任何Actor通过阵营ID查询到对应的PlayerState
	//ATopDown_GameState* GetPlayerStateByFactionID(int32 FactionID) const;
};
