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
    // ֻ�з�������Ҫ���͸������Map
    virtual void BeginPlay() override;



    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// һ�������ĺ����������κ�Actorͨ����ӪID��ѯ����Ӧ��PlayerState
	//ATopDown_GameState* GetPlayerStateByFactionID(int32 FactionID) const;
};
