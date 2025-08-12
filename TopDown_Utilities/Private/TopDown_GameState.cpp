// Fill out your copyright notice in the Description page of Project Settings.


#include "TopDown_GameState.h"
#include "TopDown_PlayerState.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

void ATopDown_GameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void ATopDown_GameState::BeginPlay()
{
    Super::BeginPlay();


    /*
    // 这段逻辑只在服务器上运行
    if (HasAuthority())
    {
        // 1. 遍历所有已经存在的PlayerState（这些是真人玩家）
        for (APlayerState* PS : PlayerArray)
        {
            if (ATopDown_GameState* FactionPS = Cast<ATopDown_GameState>(PS))
            {
                // 假设玩家的FactionID已经通过其他方式设置好了
                // 你可能需要在玩家登录或选择阵营时设置这个ID
                // 这里我们暂时假设玩家1的ID是1
                FactionPS->SetFactionId(1); 
                FactionPlayerStates.Add(1, FactionPS);
            }
        }

        // 2. 为AI阵营创建“虚拟”的PlayerState
        // 假设我们的游戏最多有4个阵营，阵营1是玩家，2/3/4是AI
        for (int32 FactionID = 2; FactionID <= 4; ++FactionID)
        {
            // 检查这个AI阵营是否已经有PlayerState了（不太可能，但保险起见）
            if (!FactionPlayerStates.Contains(FactionID))
            {
                // 生成一个新的PlayerState
                ATopDown_GameState* AI_PS = GetWorld()->SpawnActor<ATopDown_GameState>();
                if (AI_PS)
                {
                    // 为它设置阵营ID（你需要在PlayerState中添加SetFactionId函数）
                    // AI_PS->SetFactionId(FactionID); 
                    // 将它添加到我们的Map中
                    FactionPlayerStates.Add(FactionID, AI_PS);
                    UE_LOG(LogTemp, Warning, TEXT("GameState: Created and registered virtual PlayerState for AI Faction %d"), FactionID);
                }
            }
        }
    }*/
}
