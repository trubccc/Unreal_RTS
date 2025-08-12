// Fill out your copyright notice in the Description page of Project Settings.


#include "TopDown_PlayerState.h"
#include "AbilitySystemComponent.h"
#include "ResourceData.h"
#include "InteractableBase.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Resource_AttributeSet.h"



ATopDown_PlayerState::ATopDown_PlayerState()
{
    AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    AbilitySystemComponent->SetIsReplicated(true);
    AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	ResourceAttributeSet = CreateDefaultSubobject<UResource_AttributeSet>(TEXT("ResourceAttributeSet"));

    //bind attributes change events
    if(AbilitySystemComponent)
    {
        AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(ResourceAttributeSet->GetWoodAttribute()).AddUObject(this, &ATopDown_PlayerState::WoodCountChanged);
        AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(ResourceAttributeSet->GetStoneAttribute()).AddUObject(this, &ATopDown_PlayerState::StoneCountChanged);
        AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(ResourceAttributeSet->GetGoldAttribute()).AddUObject(this, &ATopDown_PlayerState::GoldCountChanged);
        AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(ResourceAttributeSet->GetFoodAttribute()).AddUObject(this, &ATopDown_PlayerState::FoodCountChanged);
    }
}   

void ATopDown_PlayerState::BeginPlay()
{   
    Super::BeginPlay();
    UE_LOG(LogTemp, Warning, TEXT("PlayerState BeginPlay"));
    OnResourcesCountChanged();

    // 监听资源采集消息
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
	MessageSubsystem.RegisterListener<FResourceHarvestedMessage>(FGameplayTag::RequestGameplayTag(FName("Resource.Harvested")), this, &ATopDown_PlayerState::OnResourceHarvested);
}

void ATopDown_PlayerState::WoodCountChanged(const FOnAttributeChangeData& ChangeData)
{
    OnResourcesCountChanged();
}

void ATopDown_PlayerState::StoneCountChanged(const FOnAttributeChangeData& ChangeData)
{
    OnResourcesCountChanged();
}

void ATopDown_PlayerState::GoldCountChanged(const FOnAttributeChangeData& ChangeData)
{
    OnResourcesCountChanged();
}

void ATopDown_PlayerState::FoodCountChanged(const FOnAttributeChangeData& ChangeData)
{
    OnResourcesCountChanged();
}

void ATopDown_PlayerState::OnResourcesCountChanged()
{
    UE_LOG(LogTemp, Warning, TEXT("OnResourcesCountChanged"));
    
    //Message Tag
    FGameplayTag MessageTag = FGameplayTag::RequestGameplayTag(FName("Resource.Update"));

    //Create a new message
    FResourceMessage ResourceMessage;
    TArray<FGameplayAttribute>& AllResourcesAttribute = ResourceMessage.ResourcesAttribute;
    AbilitySystemComponent->GetAllAttributes(AllResourcesAttribute);

    //Broadcast the message
    UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
    MessageSubsystem.BroadcastMessage<FResourceMessage>(MessageTag, ResourceMessage);
}

bool ATopDown_PlayerState::CanBuildCast(const TMap<FGameplayAttribute,int32>& BuildCast)
{
    if (!AbilitySystemComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerState: Cannot check cost because AbilitySystemComponent is NULL!"));
        return false;
    }

    for(const auto& Cost : BuildCast)
    {
        FGameplayAttribute Attribute = Cost.Key;
        int32 RequiredAmount = Cost.Value;

        //check if enough
        // 检查当前属性值是否小于所需数量
        if(AbilitySystemComponent->GetNumericAttribute(Attribute) < RequiredAmount)
        {   
            UE_LOG(LogTemp, Warning, TEXT("Cannot afford build cost: Not enough %s."), *Attribute.GetName());
            return false; // 只要有一种资源不够，就立刻返回false
        }
        /*
        float CurrenValue = AbilitySystemComponent->GetNumericAttribute(Attribute);
        if(CurrenValue < RequiredAmount)
        {   
            //no enough
            return false;
        }
        */
    }
    /*
    for(const auto& Cost : BuildCast)
    {
        FGameplayAttribute Attribute = Cost.Key;
        int32 RequiredAmount = Cost.Value;
        //deduct the cast
        AbilitySystemComponent->SetNumericAttributeBase(Attribute, AbilitySystemComponent->GetNumericAttribute(Attribute) - RequiredAmount);

    } 
    OnResourcesCountChanged();
    */
    UE_LOG(LogTemp, Warning, TEXT("PlayerState: All resource checks passed. Can afford."));
    return true;
}

void ATopDown_PlayerState::SpendBuildCost(const TMap<FGameplayAttribute, int32>& BuildCost)
{
    if (!AbilitySystemComponent) return;

    for(const auto& CostPair : BuildCost)
    {
        FGameplayAttribute Attribute = CostPair.Key;
        int32 RequiredAmount = CostPair.Value;
        
        // 使用ApplyModToAttribute来扣除，这样可以触发更多的GAS事件，更健壮
        AbilitySystemComponent->ApplyModToAttribute(Attribute, EGameplayModOp::Additive, -RequiredAmount);
    } 
    
    // OnResourcesCountChanged(); // ApplyModToAttribute 应该会自动触发属性变化代理，这里可能不需要手动调用
}

void ATopDown_PlayerState::RevertBuildCast(const TMap<FGameplayAttribute, int32> &BuildCast)
{
    for(const auto& Cost : BuildCast)
    {
        FGameplayAttribute Attribute = Cost.Key;
        int32 AmountToRevert = Cost.Value;
        //deduct the cast
        AbilitySystemComponent->SetNumericAttributeBase(Attribute, AbilitySystemComponent->GetNumericAttribute(Attribute) + AmountToRevert);

    } 
    OnResourcesCountChanged();
}

UAbilitySystemComponent* ATopDown_PlayerState::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}


void ATopDown_PlayerState::OnResourceHarvested(FGameplayTag Channel, const FResourceHarvestedMessage& Message)
{
	if (!Message.Harvester) return;

    // 1. 先尝试将 AActor* 转换为 APawn*
	APawn* HarvesterPawn = Cast<APawn>(Message.Harvester);

    // 2. 确保转换成功，并且它的 PlayerState 是我们自己
	if (!HarvesterPawn || HarvesterPawn->GetPlayerState() != this)
    {
        return;
    }

	// 这里你需要使用GAS的Attribute来修改资源，而不是直接修改WoodCount
	if (Message.ResourceType == FGameplayTag::RequestGameplayTag(FName("ResourceType.Wood")))
	{
		// WoodCount += Message.Amount; // 旧方法
        // 新方法：应用一个GameplayEffect来增加属性值
        // 或者直接修改属性
        AbilitySystemComponent->ApplyModToAttribute(ResourceAttributeSet->GetWoodAttribute(), EGameplayModOp::Additive, Message.Amount);

	}
	else if (Message.ResourceType == FGameplayTag::RequestGameplayTag(FName("ResourceType.Stone")))
	{
		// StoneCount += Message.Amount; // 旧方法
        AbilitySystemComponent->ApplyModToAttribute(ResourceAttributeSet->GetStoneAttribute(), EGameplayModOp::Additive, Message.Amount);
	}
	
	UE_LOG(LogTemp, Log, TEXT("Player %s harvested resources."), *GetName());
}

