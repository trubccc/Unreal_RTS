// Fill out your copyright notice in the Description page of Project Settings.


#include "AN_AttackFinished.h"
#include "CLMBasePawn.h"

void UAN_AttackFinished::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);
    if (ACLMBasePawn* Pawn = Cast<ACLMBasePawn>(MeshComp->GetOwner()))
    {
        //Pawn->AnimationFinished(Animation);
        Pawn->AttackAnimationFinished();
    }

}
