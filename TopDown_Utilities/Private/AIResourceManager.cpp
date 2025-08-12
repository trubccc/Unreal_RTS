// Fill out your copyright notice in the Description page of Project Settings.


#include "AIResourceManager.h"
#include "AbilitySystemComponent.h"
#include "Kismet/GameplayStatics.h"


AAIResourceManager* AAIResourceManager::ResourceManagerInstance = nullptr;
// Sets default values
AAIResourceManager::AAIResourceManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AAIResourceManager::BeginPlay()
{
	Super::BeginPlay();
	ResourceManagerInstance = this;
}

// Called every frame
void AAIResourceManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// ��ȡ����ʵ���ĺ���
AAIResourceManager* AAIResourceManager::GetInstance(UWorld* World)
{
    if (!ResourceManagerInstance)
    {
        // ���ʵ�������ڣ��ͳ������������ҵ���
        ResourceManagerInstance = Cast<AAIResourceManager>(UGameplayStatics::GetActorOfClass(World, AAIResourceManager::StaticClass()));
        if (!ResourceManagerInstance)
        {
            UE_LOG(LogTemp, Error, TEXT("AIResourceManager instance not found in the world!"));
        }
    }
    return ResourceManagerInstance;
}

// ΪAI��Ӫ������Դ�ĺ����߼�
void AAIResourceManager::AddResource(int32 FactionID, TSubclassOf<UGameplayEffect> ResourceEffectClass)
{
    // ��������Ӫ��û����Դ��¼����Ϊ������һ��
    if (!FactionResources.Contains(FactionID))
    {
        UResource_AttributeSet* NewAttributeSet = NewObject<UResource_AttributeSet>(this);
        FactionResources.Add(FactionID, NewAttributeSet);
        UE_LOG(LogTemp, Warning, TEXT("AIResourceManager: Created new resource set for Faction %d"), FactionID);
    }

    // ��ȡ����Ӫ�����Լ�
    UResource_AttributeSet* AttributeSet = FactionResources[FactionID];

    // --- ģ��Ӧ��GameplayEffect ---
    // ��ΪAIû��������ASC������ֱ���޸�����ֵ
    if (AttributeSet && ResourceEffectClass)
    {
        // ��ȡGE��Ĭ�϶����Զ�ȡ�������η���Ϣ
        UGameplayEffect* EffectCDO = ResourceEffectClass->GetDefaultObject<UGameplayEffect>();
        for (const FGameplayModifierInfo& ModInfo : EffectCDO->Modifiers)
        {
            float Magnitude = 0.f;
            ModInfo.ModifierMagnitude.GetStaticMagnitudeIfPossible(1, Magnitude);

            // �򵥵ؼ�������� "Add"
            if (ModInfo.Attribute == UResource_AttributeSet::GetWoodAttribute())
            {
                AttributeSet->SetWood(AttributeSet->GetWood() + Magnitude);
                UE_LOG(LogTemp, Warning, TEXT("Faction %d Wood: %f"), FactionID, AttributeSet->GetWood());
            }
            else if (ModInfo.Attribute == UResource_AttributeSet::GetStoneAttribute())
            {
                AttributeSet->SetStone(AttributeSet->GetStone() + Magnitude);
                UE_LOG(LogTemp, Warning, TEXT("Faction %d Stone: %f"), FactionID, AttributeSet->GetStone());
            }
            else if (ModInfo.Attribute == UResource_AttributeSet::GetFoodAttribute())
            {
                AttributeSet->SetFood(AttributeSet->GetFood() + Magnitude);
                UE_LOG(LogTemp, Warning, TEXT("Faction %d Food: %f"), FactionID, AttributeSet->GetFood());
            }
            else if (ModInfo.Attribute == UResource_AttributeSet::GetGoldAttribute())
            {
                AttributeSet->SetGold(AttributeSet->GetGold() + Magnitude);
                UE_LOG(LogTemp, Warning, TEXT("Faction %d Gold: %f"), FactionID, AttributeSet->GetGold());
            }
            // ... ������Ϊ������Դ���� (Food, Gold) ���� else if
        }
    }
}

