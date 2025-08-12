// Fill out your copyright notice in the Description page of Project Settings.


#include "AN_GatherHit.h"
#include "CLMBasePawn.h"

void UAN_GatherHit::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    // 调用父类的函数，这是一个好习惯
    Super::Notify(MeshComp, Animation, EventReference);

    // 尝试获取动画的拥有者，并将其转换为我们的Pawn类型
    if (ACLMBasePawn* Pawn = Cast<ACLMBasePawn>(MeshComp->GetOwner()))
    {
        // 如果转换成功，直接调用Pawn身上我们准备好的函数
        Pawn->OnGatherHit();
    }
}