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
    // ����߼�ֻ�ڷ�����������
    if (HasAuthority())
    {
        // 1. ���������Ѿ����ڵ�PlayerState����Щ��������ң�
        for (APlayerState* PS : PlayerArray)
        {
            if (ATopDown_GameState* FactionPS = Cast<ATopDown_GameState>(PS))
            {
                // ������ҵ�FactionID�Ѿ�ͨ��������ʽ���ú���
                // �������Ҫ����ҵ�¼��ѡ����Ӫʱ�������ID
                // ����������ʱ�������1��ID��1
                FactionPS->SetFactionId(1); 
                FactionPlayerStates.Add(1, FactionPS);
            }
        }

        // 2. ΪAI��Ӫ���������⡱��PlayerState
        // �������ǵ���Ϸ�����4����Ӫ����Ӫ1����ң�2/3/4��AI
        for (int32 FactionID = 2; FactionID <= 4; ++FactionID)
        {
            // ������AI��Ӫ�Ƿ��Ѿ���PlayerState�ˣ���̫���ܣ������������
            if (!FactionPlayerStates.Contains(FactionID))
            {
                // ����һ���µ�PlayerState
                ATopDown_GameState* AI_PS = GetWorld()->SpawnActor<ATopDown_GameState>();
                if (AI_PS)
                {
                    // Ϊ��������ӪID������Ҫ��PlayerState�����SetFactionId������
                    // AI_PS->SetFactionId(FactionID); 
                    // ������ӵ����ǵ�Map��
                    FactionPlayerStates.Add(FactionID, AI_PS);
                    UE_LOG(LogTemp, Warning, TEXT("GameState: Created and registered virtual PlayerState for AI Faction %d"), FactionID);
                }
            }
        }
    }*/
}
