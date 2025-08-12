// Fill out your copyright notice in the Description page of Project Settings.


#include "AN_EnableWeaponCollision.h"
#include "CLMBasePawn.h"

void UAN_EnableWeaponCollision::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (ACLMBasePawn* OwnerPawn = Cast<ACLMBasePawn>(MeshComp->GetOwner()))
	{
		OwnerPawn->EnableWeaponCollision();
	}    
}

void UAN_EnableWeaponCollision::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (ACLMBasePawn* OwnerPawn = Cast<ACLMBasePawn>(MeshComp->GetOwner()))
	{
		OwnerPawn->DisableWeaponCollision();
	}    
}
