// Fill out your copyright notice in the Description page of Project Settings.


#include "TopDown_PlayerState.h"
#include "AbilitySystemComponent.h"
#include "ResourceData.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Resource_AttributeSet.h"



ATopDown_PlayerState::ATopDown_PlayerState()
{
    AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

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
    for(const auto& Cost : BuildCast)
    {
        FGameplayAttribute Attribute = Cost.Key;
        int32 RequiredAmount = Cost.Value;
        //check if enough
        float CurrenValue = AbilitySystemComponent->GetNumericAttribute(Attribute);
        if(CurrenValue < RequiredAmount)
        {   
            //no enough
            return false;
        }
    }

    for(const auto& Cost : BuildCast)
    {
        FGameplayAttribute Attribute = Cost.Key;
        int32 RequiredAmount = Cost.Value;
        //deduct the cast
        AbilitySystemComponent->SetNumericAttributeBase(Attribute, AbilitySystemComponent->GetNumericAttribute(Attribute) - RequiredAmount);

    } 

    OnResourcesCountChanged();
    return true;
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
