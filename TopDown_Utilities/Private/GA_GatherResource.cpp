// Fill out your copyright notice in the Description page of Project Settings.

#include "GA_GatherResource.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "ResourceInterface.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "InteractableBase.h" 
#include "ResourceData.h"

UGA_GatherResource::UGA_GatherResource()
{
	// 可以在这里设置一些默认值
}
void UGA_GatherResource::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 从触发事件中获取目标
	if (TriggerEventData && TriggerEventData->Target)
	{
		// 1. 从 const TObjectPtr 获取 const AActor*
		const AActor* ConstTarget = TriggerEventData->Target.Get();
		// 2. 使用 const_cast 移除原始指针的 const 属性
		TargetResource = const_cast<AActor*>(ConstTarget);
	}

	if (!TargetResource)
	{
		UE_LOG(LogTemp, Error, TEXT("UGA_GatherResource: TargetResource is null."));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!GatherMontage)
	{
		UE_LOG(LogTemp, Error, TEXT("UGA_GatherResource: GatherMontage is not set."));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, GatherMontage);
	if (!MontageTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	MontageTask->OnCompleted.AddDynamic(this, &UGA_GatherResource::OnAnimationFinished);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_GatherResource::OnAnimationFinished); // 动画被打断也算结束
	MontageTask->OnCancelled.AddDynamic(this, &UGA_GatherResource::OnAnimationFinished);
	MontageTask->ReadyForActivation();
}

void UGA_GatherResource::OnAnimationFinished()
{
	if (HasAuthority(&CurrentActivationInfo))
	{
		// 尝试将目标转换为InteractableBase
		if (AInteractableBase* ResourceActor = Cast<AInteractableBase>(TargetResource))
		{
			FGameplayTag MessageTag = FGameplayTag::RequestGameplayTag(FName("Resource.Harvested"));
    
			FResourceHarvestedMessage Message;
			Message.Harvester = GetAvatarActorFromActorInfo();
			Message.Resource = TargetResource;
			
			// 从目标身上获取采集数量和类型
            Message.Amount = ResourceActor->GetHarvestAmount(); 
            // 如果消息体需要，也可以在这里加入资源类型
            Message.ResourceType = ResourceActor->GetResourceTypeTag();

			UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
			MessageSubsystem.BroadcastMessage(MessageTag, Message);

			ResourceActor->Interact(GetAvatarActorFromActorInfo());
		}
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}