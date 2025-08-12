// Fill out your copyright notice in the Description page of Project Settings.


#include "CLMBasePawn.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameplayMessageStructures.h"
#include "AIController.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimInstance.h"
#include "TimerManager.h"
#include "AbilitySystemComponent.h"
#include "TopDown_PlayerState.h"
#include "TopDown_GameState.h"
#include "CombatInterface.h"
#include "AssetRegistry/AssetRegistryModule.h" 
#include "Engine/StreamableManager.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Engine/StreamableManager.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "Engine/AssetManager.h"
#include "AIResourceManager.h"
#include "TopDown_PlayerController.h" 
#include "GameFramework/PlayerController.h"
#include "BrainComponent.h"

// Sets default values
ACLMBasePawn::ACLMBasePawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationPitch = false;

	//Create Capsule 
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	SetRootComponent(CapsuleComponent);
	CapsuleComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);

	//Create Skeletal Mesh
	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(CapsuleComponent);

	//Create Movement
	FloatPawnMovement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("FloatPawnMovement"));
	FloatPawnMovement->MaxSpeed = 600.f;

	//Create selected indicator
	SelectedIndicator = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SelectedIndicator"));
	SelectedIndicator->SetupAttachment(CapsuleComponent);
	SelectedIndicator->SetHiddenInGame(true);
	SelectedIndicator->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//Network Replication
	bReplicates = true;
	SetReplicateMovement(true);
	
	// 初始化生命值
	CurrentHealth = MaxHealth;
	bIsDead = false; // 初始化为false
	bIsInHitStun = false;

	WeaponCollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("WeaponCollisionSphere"));
	WeaponCollisionSphere->SetupAttachment(SkeletalMesh); // 先附加到骨骼网上
	
	WeaponCollisionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

	WeaponCollisionSphere->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
		
	WeaponCollisionSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	WeaponCollisionSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	WeaponCollisionSphere->SetSimulatePhysics(false);
	WeaponCollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 默认禁用碰撞！非常重要！
	//WeaponCollisionSphere->SetCollisionObjectType(ECC_WorldDynamic);
	//WeaponCollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	//WeaponCollisionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap); // 只与Pawn发生重叠事件

	ToolMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ToolMeshComponent"));
	ToolMeshComponent->SetupAttachment(SkeletalMesh, FName("hand_r_socket")); // 假设你手部有一个插槽
	ToolMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	AIControllerClass = AAIController::StaticClass();	
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	
}

void ACLMBasePawn::BeginPlay()
{
	Super::BeginPlay();
	
	//Bind Mouse Event
	CapsuleComponent->OnBeginCursorOver.AddDynamic(this, &ACLMBasePawn::OnCapsuleBeginCursorOver);
	CapsuleComponent->OnEndCursorOver.AddDynamic(this, &ACLMBasePawn::OnCapsuleEndCursorOver);

    // 只有服务器需要初始化生命值
    if (HasAuthority())
    {
        CurrentHealth = MaxHealth;
		WeaponCollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &ACLMBasePawn::OnWeaponOverlap);
		//ClearState();
		SetMaxSpeed(WanderSpeed);
    }
}

void ACLMBasePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (HasAuthority())
	{
		UpdateAI();
	}	
	//Move();
}

void ACLMBasePawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (AAIController* AIController = Cast<AAIController>(NewController))
	{
		// 确保AI控制器使用了正确的黑板
		if (MyBlackboardAsset) // 假设你在.h中定义了 UPROPERTY(...) UBlackboardData* MyBlackboardAsset;
		{
			AIController->UseBlackboard(MyBlackboardAsset, BlackboardComponent);
		}
		
		// 运行行为树
		if (MyBehaviorTree) // 假设你在.h中定义了 UPROPERTY(...) UBehaviorTree* MyBehaviorTree;
		{
			AIController->RunBehaviorTree(MyBehaviorTree);
		}
	}
}





void ACLMBasePawn::HandleBeingAttacked(AActor* DamageCauser)
{
	if (DamageCauser == nullptr)
	{
		return; // 如果伤害来源为空，直接返回
	}
	//    只有实现了这个接口的Actor，才被我们视为一个“有效的攻击者”。
	if (DamageCauser->GetClass()->ImplementsInterface(UCombatInterface::StaticClass()))
	{
		// 2. 只有通过了检查，才将它添加到仇恨列表中。
		Attackers.Insert(DamageCauser, 0);
	}
	else
	{
		// （可选）打印一条警告日志，方便我们调试是哪个不该造成伤害的东西触发了逻辑
		UE_LOG(LogTemp, Warning, TEXT("[%s] was damaged by [%s], but it does not implement UCombatInterface. Ignoring as attacker."), *GetName(), *DamageCauser->GetName());
	}
}

void ACLMBasePawn::UpdateAI()
{
	if (!GetController())
	{
		// 如果这条日志在游戏时反复出现，说明AI控制器没有被成功赋予。
		UE_LOG(LogTemp, Error, TEXT("[%s] has NO AI CONTROLLER!"), *GetName());
		return;
	}
	if (bIsInHitStun || bIsDead)
	{
		return; // 如果在硬直中，就什么都别想，直接返回
	}
	
    AAIController* PawnAIController = Cast<AAIController>(GetController());
    if (!PawnAIController) return;

    // --- 状态机逻辑 ---
    switch (CurrentState)
    {
        case EUnitState::IdlePawn:
		{    // 如果空闲时有敌人，则自动反击
			//(你现有的自动反击逻辑可以放在这里)
			//AI自动反击


			if (!CurrentAttackTarget.IsValid() && Attackers.Num() > 0)
			{
				// 清理死亡的攻击者
				Attackers.RemoveAll([](const TWeakObjectPtr<AActor>& AttackerPtr)
					{
						// 1. 首先，检查弱指针本身是否有效
						if (!AttackerPtr.IsValid())
						{
							return true; // 无效，从数组中移除
						}
						AActor* Attacker = AttackerPtr.Get();
						// 2. 其次，检查获取到的原始指针是否为空
						if (Attacker == nullptr)
						{
							return true; // 指针为空，移除
						}
						// 3. 最后，在确保指针绝对有效后，再调用接口函数
						return ICombatInterface::Execute_IsDead(Attacker);
					});

				if (Attackers.Num() > 0)
				{
					AActor* TargetToCounterAttack = Attackers[0].Get(); // 从弱指针获取Actor
					if (TargetToCounterAttack)
					{
						PlayerCommandAttackTarget_Implementation(TargetToCounterAttack);
					}
				}
			}
			break;
		}
        case EUnitState::MovingToLocation:
            // 如果到达了目的地，就变为空闲
            // === 这里的比较现在可以正常工作了 ===
            if (PawnAIController->GetMoveStatus() == EPathFollowingStatus::Idle)
            {
                ClearState();
				RestoreAutonomousAI();
            }
            break;

        case EUnitState::MovingToGather:
        {  
            if (!TargetResourceNode.IsValid())
            {
                ClearState();
				RestoreAutonomousAI();
                break;
            }

            const float Distance = FVector::Dist(GetActorLocation(), TargetResourceNode->GetActorLocation());
            if (Distance <= GatherRange)
            {
				PawnAIController->StopMovement();
				//CurrentPathPoints.Empty();	// 停止移动                
                CurrentState = EUnitState::Gathering; 

                UE_LOG(LogTemp, Warning, TEXT("[%s] Arrived at resource. State: Gathering."), *GetName());               
                FVector DirectionToTarget = (TargetResourceNode->GetActorLocation() - GetActorLocation()).GetSafeNormal();
				FRotator TargetRotation = DirectionToTarget.Rotation();
                FRotator CleanRotation = FRotator(0.f, TargetRotation.Yaw, 0.f);
                SetActorRotation(CleanRotation);
				

				UAnimMontage* MontageToPlay = TargetResourceNode->GetGatheringMontage();
				if(MontageToPlay)
				{
					const float GatheringPlayRate = 1.5f; 
					Multicast_PlayMontage(MontageToPlay, GatheringPlayRate);
				}
                else
				{
                    // 如果资源没有设置动画，我们可能想直接触发完成事件，或者播放一个默认动画
                    UE_LOG(LogTemp, Warning, TEXT("[%s] Resource has no gathering montage set. Triggering completion directly."), *GetName());
                    OnGatherHit(); // 立即触发采集命中					
				}
               	//GetWorld()->GetTimerManager().SetTimer(GatheringTimerHandle, this, &ACLMBasePawn::OnGatheringComplete, 3.0f, false);
            }
            else
            {
                // === 更健壮的移动检查 ===
                if (PawnAIController->GetMoveStatus() != EPathFollowingStatus::Moving)
                {
                    PawnAIController->MoveToActor(TargetResourceNode.Get(), GatherRange * 0.7f);
                }
            }
        			
            break;
		} 
		case EUnitState::Gathering:
            // 这个状态是等待计时器完成，所以AI的Tick里什么都不用做
            break;	
			
		
        //case EUnitState::ReturningResource:
        //{   
			/*
            if (!ResourceDropOffBuilding.IsValid())
            {
                ClearState();
                break;
            }
			*/
			/*
            // 1. 获取目标建筑的边界框信息
            FVector BuildingOrigin;
            FVector BuildingBoxExtent;
            ResourceDropOffBuilding->GetActorBounds(false, BuildingOrigin, BuildingBoxExtent);

            // 2. 从边界框的范围(BoxExtent)中，找出最长的那条边的一半。
            //    BoxExtent.X 是宽度的一半，BoxExtent.Y 是长度的一半。
            const float LongestHalfSide = FMath::Max(BuildingBoxExtent.X, BuildingBoxExtent.Y);

            // 3. 定义我们想要的交互范围。
            //    我们希望它比最长边的一半再大一点，比如额外增加50个单位。
            //    这样可以确保Pawn停在建筑的外面，而不是试图挤到中心去。
            const float DropOffRange = LongestHalfSide + 50.0f;
			
            const float DropOffBuildingDistance = FVector::Dist(GetActorLocation(), BuildingOrigin);
            if (DropOffBuildingDistance <= DropOffRange)
            {
				UE_LOG(LogTemp, Warning, TEXT("[%s] DropOffBuildingDistance to target: %.2f, AttackRange: %.2f"), *GetName(), DropOffBuildingDistance, AttackRange);
				UE_LOG(LogTemp, Warning, TEXT("[%s] Arrived at TownHall. Task complete."), *GetName());
				PawnAIController->StopMovement();
                CurrentState = EUnitState::DroppingOffResource; // 切换到新状态
				//CurrentPathPoints.Empty();
                //ClearState();

				// 让Pawn面向房子，增加细节
                FVector DirectionToBuilding = (BuildingOrigin - GetActorLocation()).GetSafeNormal();
				FRotator TargetRotation = DirectionToBuilding.Rotation();
				FRotator CleanRotation = FRotator(0.f, TargetRotation.Yaw, 0.f);
                SetActorRotation(CleanRotation);
                
                // 播放放置资源的动画
                UE_LOG(LogTemp, Warning, TEXT("[%s] Arrived at TownHall. Playing DropOff animation."), *GetName());
                Multicast_PlayMontage(DropOffMontage);
            }
            else
            {
				UE_LOG(LogTemp, Warning, TEXT("[%s] Not at TownHall. Distance: %.2f"), *GetName(), DropOffBuildingDistance);
                // === 更健壮的移动检查 ===
                if (PawnAIController->GetMoveStatus() != EPathFollowingStatus::Moving)
                {
                    PawnAIController->MoveToActor(ResourceDropOffBuilding.Get(), DropOffRange * 0.9f);
                }
            }*/
		   	/*
            if (PawnAIController->GetMoveStatus() == EPathFollowingStatus::Idle)
            {
                // 到达随机点，切换到放置状态
                CurrentState = EUnitState::DroppingOffResource;
                
                FVector DirectionToBuilding = (ResourceDropOffBuilding->GetActorLocation() - GetActorLocation()).GetSafeNormal();
                FRotator TargetRotation = DirectionToBuilding.Rotation();
                FRotator CleanRotation = FRotator(0.f, TargetRotation.Yaw, 0.f);
                SetActorRotation(CleanRotation);
                
                UE_LOG(LogTemp, Warning, TEXT("[%s] Arrived at drop-off point. Playing DropOff animation."), *GetName());
                Multicast_PlayMontage(DropOffMontage);
            }		   
            break;*/			
		//}	
					
        //case EUnitState::DroppingOffResource:
        //{
            // 在这个状态下，我们什么都不用做。
            // 只是单纯地等待动画播放完毕。
            // 动画的结束会通过下面的 DropOffAnimationFinished 函数来处理。
        //    break;
        //}		
        // 其他状态（移动、攻击）由其各自的命令函数直接处理，
        // Tick中主要处理需要持续检查的逻辑。
		case EUnitState::MovingToAttack:
        {   
			if (!CurrentAttackTarget.IsValid())
            {
                StopAttacking();
				RestoreAutonomousAI();
                break;
            }

            if (PawnAIController->GetMoveStatus() != EPathFollowingStatus::Moving)
            {
                // 2. 到达后，我们再做一次最终的距离检查，确保没有超出攻击范围
                //    这可以防止Pawn因为找不到路而停在离目标很远的地方。
                const float FinalDistance = FVector::Dist(GetActorLocation(), CurrentAttackTarget->GetActorLocation());
                
                // 我们需要考虑双方的碰撞半径
                float MyRadius = 0.f;
                if (UCapsuleComponent* MyCapsule = Cast<UCapsuleComponent>(GetRootComponent()))
                {
                    MyRadius = MyCapsule->GetScaledCapsuleRadius();
                }
                float TargetRadius = 0.f;
                if (UCapsuleComponent* TargetCapsule = Cast<UCapsuleComponent>(CurrentAttackTarget->GetRootComponent()))
                {
                    TargetRadius = TargetCapsule->GetScaledCapsuleRadius();
                }

                // 实际的物理间距 = 中心点距离 - 双方半径
                const float ActualGap = FinalDistance - MyRadius - TargetRadius;

                if (ActualGap <= AttackRange)
                {
                    // 如果间距在攻击范围内，就开始攻击
                    CurrentState = EUnitState::Attacking;
                    CheckAndPerformAttack();
                }
                else
                {
                    // 如果间距仍然太远，说明任务失败（比如目标在悬崖上）
                    StopAttacking();
                }
            }

			/*
            const float AttackTargetDistance = FVector::Dist(GetActorLocation(), CurrentAttackTarget->GetActorLocation());

            // === 距离判断 ===
            if (AttackTargetDistance <= AttackRange)
            {
                // 到达范围，停止移动，并切换到攻击状态
				//CurrentPathPoints.Empty(); 
                PawnAIController->StopMovement();
                CurrentState = EUnitState::Attacking;
                // 为了立即攻击，直接调用一次CheckAndPerformAttack
                CheckAndPerformAttack(); 
            }
            else
            {
                // 不在范围，继续移动
                // 确保AI正在向目标移动
				UE_LOG(LogTemp, Warning, TEXT("[%s] Not at. Distance: %.2f"), *GetName(), AttackTargetDistance);
                if (PawnAIController->GetMoveStatus() != EPathFollowingStatus::Moving)
                {
                    PawnAIController->MoveToActor(CurrentAttackTarget.Get(), AttackRange * 0.7f);

                }
            }
			*/
            break;
		}
        case EUnitState::Attacking:

			CheckAndPerformAttack();
			/*
			// --- 优先级1：玩家指令 ---
			if (CommandedAttackTarget.IsValid())
			{
				if (ICombatInterface::Execute_IsDead(CommandedAttackTarget.Get()))
				{
					StopAttacking();s
					return;
				}
				
				// 如果当前没有在攻击，也没有在走向目标，就尝试攻击
				if (!bIsAttacking && !GetWorld()->GetTimerManager().IsTimerActive(AttackTimerHandle))
				{
					CheckAndPerformAttack();
				}
				return;
			}
			*/	
			break;

        

    }





}

// Called when the game starts or when spawned



void ACLMBasePawn::Move()
{
	if(!bMoving)
	{
		return;
	}

	//Get direction to move
	FVector MoveDirection = (MoveTargerLocation - GetActorLocation());

	//Move in that direction
	if(MoveDirection.Length() < AcceptanceRadius)
	{
		bMoving = false;
		return;
	}

	MoveDirection.Normalize(1);
	//AddMovementInput(MoveDirection, 1.f);

	FRotator DirectRotation = UKismetMathLibrary::MakeRotFromX(MoveDirection);
	DirectRotation.Pitch = 0;
	DirectRotation.Roll = 0;

	FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), DirectRotation, GetWorld()->DeltaTimeSeconds, TurnSpeed);
	SetActorRotation(NewRotation);

}

// Called every frame
void ACLMBasePawn::OnWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;

	UE_LOG(LogTemp, Warning, TEXT("[%s] Weapon Overlapped with [%s]"), *GetName(), *OtherActor->GetName());

    // 确保我们命中的是我们想要攻击的目标，并且不是自己
	if (OtherActor && OtherActor == CurrentAttackTarget.Get())
	{
		if (OtherActor->GetClass()->ImplementsInterface(UCombatInterface::StaticClass()))
		{
			// 对目标造成伤害
			ICombatInterface::Execute_TakeDamage(OtherActor, AttackDamage, this);

			// --- 关键：在一次成功的攻击后立刻禁用碰撞 ---
			// 这可以防止一次挥拳（一个动画）对同一个目标造成多次伤害
			DisableWeaponCollision();
		}
	}	
}

// Called to bind functionality to input
void ACLMBasePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ACLMBasePawn::SelectedActorLocal(const bool IsSelected)
{
	SelectedIndicator->SetHiddenInGame(!IsSelected);
}

void ACLMBasePawn::SelectedActor_Implementation(const bool Selected)
{
	SelectedIndicator->SetHiddenInGame(!Selected);
}

void ACLMBasePawn::MoveToLocation_Implementation(const FVector TargetLocation)
{
	AAIController* PawnAIController = Cast<AAIController>(GetController());
    // 只有服务器能执行移动逻辑
    if (HasAuthority())
    {	/*	
		if (PawnAIController && PawnAIController->GetBrainComponent())
		{
			PawnAIController->GetBrainComponent()->StopLogic("New player command received");
		}
		*/
		ClearState();// 这一行已经包含了停止攻击、清空目标等所有操作
		CurrentState = EUnitState::MovingToLocation;
		//RequestPathTo(TargetLocation);
	    //StopAttacking();
		//Attackers.Empty();
		//CommandedAttackTarget = nullptr;
    }
	
	UE_LOG(LogTemp, Display, TEXT("Navigating"));

	MoveTargerLocation = TargetLocation + FVector(0, 0, GetDefaultHalfHeight());
	bMoving = true;
	
	if(PawnAIController)
	{
		SetMaxSpeed(CommandedSpeed);
		PawnAIController->MoveToLocation(MoveTargerLocation, AcceptanceRadius);
	}
	
}

EActorType ACLMBasePawn::GetActorType_Implementation()
{
    return PawnType;
}

/*
TArray<TSubclassOf<ACLMBaseBuilding>> ACLMBasePawn::GetBuildingOptions_Implementation()
{
    return BuildingOptions;
}
*/
void ACLMBasePawn::SetFaction_Implementation(int32 Newfaction)
{
	FactionID = Newfaction;
}

int32 ACLMBasePawn::GetFaction_Implementation()
{
    return FactionID;
}

void ACLMBasePawn::OnHover_Implementation()	
{
	//UE_LOG(LogTemp, Warning, TEXT("OnHover_Implementation Triggered"));
	APlayerController* LocalPlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if(LocalPlayerController)
	{
		LocalPlayerController->CurrentMouseCursor = this->IsEnemy ? EMouseCursor::Crosshairs : EMouseCursor::Hand;
	}
}


void ACLMBasePawn::ClearHover_Implementation()
{	
	//UE_LOG(LogTemp, Warning, TEXT("ClearHover_Implementation Triggered"));

	APlayerController* LocalPlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if(LocalPlayerController)
	{
		LocalPlayerController->CurrentMouseCursor = EMouseCursor::Default;
	}
}

void ACLMBasePawn::OnCapsuleBeginCursorOver(UPrimitiveComponent* TouchedComponent)
{
	//UE_LOG(LogTemp, Display, TEXT("OnCapsuleBeginCursorOver"));

	//If we are not the local player, do nothing

	//Gameplay messsage tag
	FGameplayTag MessageTag = FGameplayTag::RequestGameplayTag(FName("Actor.Events.BeginCursorOver"));

	//Create message
	FCommonGamePlayMessage Message;
	Message.Sender = this;

	OnHover_Implementation();

	//Broadcast the message
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	MessageSubsystem.BroadcastMessage(MessageTag, Message);		


}

void ACLMBasePawn::OnCapsuleEndCursorOver(UPrimitiveComponent* TouchedComponent)
{
	//UE_LOG(LogTemp, Display, TEXT("OnCapsuleEndCursorOver"));
	//If we are not the local player, do nothing

	//Gameplay messsage tag
	FGameplayTag MessageTag = FGameplayTag::RequestGameplayTag(FName("Actor.Events.EndCursorOver"));

	//Create message
	FCommonGamePlayMessage Message;
	Message.Sender = this;

	ClearHover_Implementation();
	//Broadcast the message
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	MessageSubsystem.BroadcastMessage(MessageTag, Message);		


}	

// 注册需要网络复制的变量
void ACLMBasePawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACLMBasePawn, CurrentHealth);
	DOREPLIFETIME(ACLMBasePawn, bIsAttacking);
	DOREPLIFETIME(ACLMBasePawn, bIsDead);
	DOREPLIFETIME(ACLMBasePawn, CurrentState);
}

// 当CurrentHealth在客户端上被更新时，这个函数会被调用
void ACLMBasePawn::OnRep_CurrentHealth()
{
	//更新血条UI等

}


void ACLMBasePawn::CommandAttackTarget_Implementation(AActor* TargetToAttack)
{
	// 只有服务器能下达攻击命令
	if (!HasAuthority()) return;

	CurrentAttackTarget = TargetToAttack;
    
    // 如果目标有效，设置一个重复计时器来不断检查攻击状态
	if (CurrentAttackTarget.IsValid())
	{	
		/*
		ACLMBasePawn* TargetPawn = Cast<ACLMBasePawn>(TargetToAttack);
		if(TargetPawn)
		{
			const float SelfRadius = CapsuleComponent->GetScaledCapsuleRadius();
			const float TargetRadius = TargetPawn->CapsuleComponent->GetScaledCapsuleRadius();

			CurrentEngageRange = SelfRadius + TargetRadius + 10.0f;
		}
		else
		{
			CurrentEngageRange = AttackRange;
		}
		GetWorld()->GetTimerManager().SetTimer(AttackTimerHandle, this, &ACLMBasePawn::CheckAndPerformAttack, 0.2f, true);
		*/
		CheckAndPerformAttack();
	}
	else
	{
		StopAttacking();
	}
}


void ACLMBasePawn::TakeDamage_Implementation(float DamageAmount, AActor* DamageCauser)
{
	if (!HasAuthority()) return;
	UE_LOG(LogTemp, Warning, TEXT("[%s] is taking %.2f damage from [%s]"), *GetName(), DamageAmount, *DamageCauser->GetName());
	if (bIsDead) return;
	
	if (bIsAttacking)
	{
		ResetAttackState();
	}
	
	ClearState();
	bIsInHitStun = true;

    // 停止所有当前的AI移动和攻击意图，因为我们被击中了
	
	//StopAttacking();
	AAIController* PawnAIController = Cast<AAIController>(GetController());
	if (PawnAIController)
	{
		PawnAIController->StopMovement();
	}

	if(DamageCauser)
	{
		if (!CurrentAttackTarget.IsValid())
		{
			const FVector DirectionToCauser = (GetActorLocation() - DamageCauser->GetActorLocation()).GetSafeNormal();
			const FRotator LookAtRotation = DirectionToCauser.Rotation();
			SetActorRotation(FRotator(0.f, LookAtRotation.Yaw + 180.f, 0.f));	
		}
	
	}
	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.0f, MaxHealth);
    // 检查Pawn是否由AI控制
    if (AAIController* AIController = Cast<AAIController>(GetController()))
    {
        // 获取大脑组件 (BrainComponent)，行为树是由它运行的
        if (UBrainComponent* BrainComp = AIController->GetBrainComponent())
        {
            // 如果行为树当前正在运行，就暂停它
            if (BrainComp->IsRunning())
            {
                BrainComp->PauseLogic("Taking damage"); // PauseLogic会暂停行为树的执行
                UE_LOG(LogTemp, Warning, TEXT("[%s] Behavior Tree paused due to taking damage."), *GetName());
            }
        }
    }	
    // 播放受击动画
	Multicast_PlayMontage(HitMontage, 1.0f);
	HandleBeingAttacked(DamageCauser);

    // 这个时间应该约等于你受击动画的长度
	float Duration = HitMontage ? HitMontage->GetPlayLength() * 0.9f : 0.5f; // 获取动画长度或使用默认值，这里取90%的长度，想短一点可以在这里调整
	const float HitStunDuration = FMath::Min(Duration, 0.4f); // 取计算出的时长和0.4秒中的较小值
	GetWorld()->GetTimerManager().SetTimer(
		HitStunTimerHandle,
		this,
		&ACLMBasePawn::EndHitStun,
		HitStunDuration,
		false
	);

	if (CurrentHealth <= 0.0f)
	{
		Die();
	}
}

void ACLMBasePawn::PlayerCommandAttackTarget_Implementation(AActor* TargetToAttack)
{
	if (!HasAuthority()) return;

    if (TargetToAttack == CurrentAttackTarget.Get() && (CurrentState == EUnitState::MovingToAttack || CurrentState == EUnitState::Attacking))
    {
        // 打印一条日志，方便我们知道指令被忽略了
        UE_LOG(LogTemp, Log, TEXT("[%s] Received redundant attack command on the same target. Ignoring."), *GetName());
        return;
    }
	
	ClearState();
    // 玩家的指令具有最高优先级
    // 1. 清空所有AI驱动的仇恨
	//Attackers.Empty();
    // 2. 将目标同时设置为“玩家指定目标”和“当前攻击目标”
	CommandedAttackTarget = TargetToAttack;

	CurrentAttackTarget = TargetToAttack;
	CurrentState = EUnitState::MovingToAttack; 
	//RequestPathTo(TargetToAttack);

	
	AAIController* PawnAIController = Cast<AAIController>(GetController());
	if(PawnAIController)
	{
		SetMaxSpeed(CommandedSpeed);
		//PawnAIController->MoveToActor(CurrentAttackTarget.Get(), AttackRange * 0.9f);
		PawnAIController->MoveToActor(CurrentAttackTarget.Get(), AttackRange * 0.6f);

	}
	
	//CommandAttackTarget_Implementation(TargetToAttack); // 调用通用的攻击函数来启动攻击循环
}

void ACLMBasePawn::AttackAnimationFinished()
{
	ResetAttackState();
	// --- 在这里安排下一次攻击 ---
	if (HasAuthority() && CurrentAttackTarget.IsValid())
	{
		// 检查目标是否还活着
		if (!ICombatInterface::Execute_IsDead(CurrentAttackTarget.Get()))
		{
			// 根据AttackRate设置一个一次性的计时器，在冷却结束后再次调用CheckAndPerformAttack
			GetWorld()->GetTimerManager().SetTimer(
				AttackTimerHandle,
				this,
				&ACLMBasePawn::CheckAndPerformAttack,
				AttackRate, // 使用我们定义的可配置攻击间隔！
				false       // false表示这不是一个重复的计时器
			);
		}
		else
		{
			// 如果目标在动画播放期间死了，就停止攻击
			StopAttacking();
		}
	}	
}

void ACLMBasePawn::EnableWeaponCollision()
{
	if (HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Weapon Collision ENABLED"), *GetName());
		WeaponCollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}	
}

void ACLMBasePawn::DisableWeaponCollision()
{
	if (HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Weapon Collision DISABLED"), *GetName());
		WeaponCollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}	
}

// 检查距离并攻击
void ACLMBasePawn::CheckAndPerformAttack()
{
	UE_LOG(LogTemp, Warning, TEXT("[%s] CheckAndPerformAttack executing."), *GetName());
	// 确保这段代码只在服务器上运行
	if (!HasAuthority() || !CurrentAttackTarget.IsValid())
	{	
        UE_LOG(LogTemp, Warning, TEXT("[%s] Target is invalid or no authority, stopping attack."), *GetName());
		StopAttacking();
		return;
	}

    // 检查目标是否实现了战斗接口
	if (CurrentAttackTarget->GetClass()->ImplementsInterface(UCombatInterface::StaticClass()))
	{
        // 调用接口函数来查询死亡状态
		if (ICombatInterface::Execute_IsDead(CurrentAttackTarget.Get()))
		{
            // 如果目标已死，停止攻击并返回
			StopAttacking();
			return;
		}
	}
    // --- 死亡状态检查结束 ---

	if (bIsAttacking)
	{
		return;
	}

	const float Distance = FVector::Dist(GetActorLocation(), CurrentAttackTarget->GetActorLocation());
	AAIController* PawnAIController = Cast<AAIController>(GetController());	

	UE_LOG(LogTemp, Warning, TEXT("[%s] Distance to target: %.2f, AttackRange: %.2f"), *GetName(), Distance, AttackRange);

	// 如果在攻击范围内
	if (Distance <= AttackRange)
	{
		
		UE_LOG(LogTemp, Warning, TEXT("[%s] In Attack Range! Calling PerformAttack."), *GetName());
		// 停止移动，面朝敌人，然后攻击
		
		if (PawnAIController)
		{
			PawnAIController->StopMovement();

			CurrentState = EUnitState::Attacking;
			bMoving = false;

			//const FVector DirectionToTarget = (CurrentAttackTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
			FVector DirectionToTarget = (CurrentAttackTarget->GetActorLocation() - GetActorLocation());
			DirectionToTarget.Normalize(1);
			FRotator DirectRotation = UKismetMathLibrary::MakeRotFromX(DirectionToTarget);
			DirectRotation.Pitch = 0;
			DirectRotation.Roll = 0;

			FRotator LookAtRotation = FMath::RInterpTo(GetActorRotation(), DirectRotation, GetWorld()->DeltaTimeSeconds, TurnSpeed);
			SetActorRotation(LookAtRotation);

			bIsAttacking = true;
			Multicast_PlayMontage(AttackMontage, 1.0f);
			/*
			if (CurrentAttackTarget.IsValid() && CurrentAttackTarget->GetClass()->ImplementsInterface(UCombatInterface::StaticClass()))
			{
				ICombatInterface::Execute_TakeDamage(CurrentAttackTarget.Get(), AttackDamage, this);
			}
			*/
		}
		else
		{
			//CurrentState = EUnitState::MovingToAttack;
			// 如果没有AI控制器，什么也做不了
			StopAttacking();
			return;
		}
	}
	/*
	else if(Distance <= EngagePreparationRange)
	{
		// 停止AI的寻路，我们自己接管
		PawnAIController->StopMovement();
			
		// 自己控制移动，平滑地“滑”向目标
		const FVector DirectionToTarget = (CurrentAttackTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		AddMovementInput(DirectionToTarget, 1.0f);

		// 自己控制旋转，保证时刻面对目标
		const FRotator LookAtRotation = DirectionToTarget.Rotation();
		const FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), LookAtRotation, GetWorld()->GetDeltaSeconds(), TurnSpeed);
		SetActorRotation(FRotator(0.f, NewRotation.Yaw, 0.f));				
	}	

		// 让Pawn面朝目标
		//FVector DirectionToTarget = (CurrentAttackTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		//FRotator LookAtRotation = DirectionToTarget.Rotation();
		//LookAtRotation.Pitch = 0;
		//SetActorRotation(LookAtRotation);
	*/
	else // 如果不在攻击范围内
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Out of range, moving to target."), *GetName());
		// 命令AI移动到目标Actor。
		// MoveToActor会自动处理路径寻找，这是正确的做法。
		if(PawnAIController)
		{	
			CurrentState = EUnitState::MovingToAttack;
			//Move();
			//PawnAIController->MoveToActor(CurrentAttackTarget.Get(), AttackRange * 0.9f, false); // 移动到比攻击范围稍近一点的地方，防止来回抖动
			
			// 1. 获取AI当前的移动方向
			//const FVector CurrentVelocity = GetVelocity();

			FVector DirectionToTarget = (CurrentAttackTarget->GetActorLocation() - GetActorLocation());
			DirectionToTarget.Normalize(1);
			// 2. 只有在移动时才更新朝向 (速度大于0)
	
			// 3. 将速度向量转换为旋转值
			FRotator DirectRotation = UKismetMathLibrary::MakeRotFromX(DirectionToTarget);
			DirectRotation.Pitch = 0;
			DirectRotation.Roll = 0;
			// 4. 平滑地将Pawn转向移动方向
			FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), DirectRotation, GetWorld()->GetDeltaSeconds(), TurnSpeed);
				
			SetActorRotation(NewRotation);
			
			PawnAIController->MoveToActor(CurrentAttackTarget.Get());
			
		}
		
	}
}


void ACLMBasePawn::StopAttacking()
{
	GetWorld()->GetTimerManager().ClearTimer(AttackTimerHandle);
    // 如果我们当前攻击的目标就是玩家指定的目标，那么在它死后，这个指令就算完成了
	if (CurrentAttackTarget.IsValid() && CurrentAttackTarget == CommandedAttackTarget.Get())
	{
		CommandedAttackTarget = nullptr;
	}	
	CurrentAttackTarget = nullptr;
	//ResetAttackState();
    // 当停止攻击时，我们不需要等待下一帧的Tick，可以立即尝试触发一次AI决策
	//UpdateAI();	

    //如果状态是攻击相关的，则将其重置为空闲
    if (CurrentState == EUnitState::Attacking || CurrentState == EUnitState::MovingToAttack)
    {
        ClearState();
		RestoreAutonomousAI();
    }
    // 检查Pawn是否由AI控制
    if (AAIController* AIController = Cast<AAIController>(GetController()))
    {
        if (UBrainComponent* BrainComp = AIController->GetBrainComponent())
        {
            // 如果行为树之前被暂停了，就恢复它的执行
            if (BrainComp->IsPaused())
            {
                BrainComp->ResumeLogic("Combat finished"); // ResumeLogic会从上次暂停的地方继续执行
                UE_LOG(LogTemp, Warning, TEXT("[%s] Behavior Tree resumed after combat."), *GetName());
            }
        }
    }
}

void ACLMBasePawn::Die()
{
	bIsDead = true;
	// 停止所有计时器和逻辑
	StopAttacking();
    
	
    // 播放死亡动画
	//Multicast_PlayMontage(DieMontage);
    Multicast_HandleDeath_Implementation();
    // 禁用碰撞，让单位像尸体一样

    // 在几秒后销毁Actor
	SetLifeSpan(5.0f);
}


void ACLMBasePawn::AnimationFinished(UAnimMontage* Montage)
{
	if (HasAuthority())
	{
        // 如果是攻击动画结束了
		if (Montage == AttackMontage)
		{
			bIsAttacking = false;
		}
	}
}
/*
void ACLMBasePawn::DropOffAnimationFinished(UAnimMontage* Montage)
{
	if (HasAuthority())
	{
        // === 处理攻击动画结束 ===
		if (Montage == AttackMontage)
		{
			bIsAttacking = false;
            // AttackAnimationFinished() 已经处理了攻击逻辑，这里可以保持原样或调用它
		}

        // === 新增：处理放置资源动画结束 ===
        if (Montage == DropOffMontage)
        {
            // 确保我们是在正确的状态下完成的
            if (CurrentState == EUnitState::DroppingOffResource)
            {
                UE_LOG(LogTemp, Warning, TEXT("[%s] Drop-off animation finished. Clearing state to Idle."), *GetName());
                // 在这里可以真正在玩家的仓库里增加资源数量
                // 检查Pawn是否“携带”了资源效果
                if (CarriedResourceEffectClass)
                {
                    // 1. 获取这个Pawn所属的玩家状态 (PlayerState)
                    // PlayerState通常持有AbilitySystemComponent
                    if (APlayerState* OwningPlayerState = GetPlayerState())
                    {
                        // 2. 从PlayerState获取AbilitySystemComponent
                        if (UAbilitySystemComponent* ASC = OwningPlayerState->FindComponentByClass<UAbilitySystemComponent>())
                        {
                            // 3. 创建一个EffectContext，它可以携带额外信息
                            FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
                            ContextHandle.AddSourceObject(this); // 将Pawn自己作为效果来源

                            // 4. 定义效果的规格 (Spec)
                            // 我们使用“暂存”的效果类，并设置等级为1
                            FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(CarriedResourceEffectClass, 1.0f, ContextHandle);

                            // 5. 如果规格有效，就将它应用到目标（在这里是ASC自己）身上
                            if (SpecHandle.IsValid())
                            {
                                ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
                                UE_LOG(LogTemp, Warning, TEXT("[%s] Applied carried resource effect to player!"), *GetName());
                            }
                        }
                    }
                }
                ClearState(); // 动画放完，任务彻底完成，变为空闲
            }
        }
	}
}
*/
// 这个函数将在攻击动画结束，或被任何方式打断时调用
void ACLMBasePawn::ResetAttackState()
{
	if (HasAuthority())
	{
		bIsAttacking = false;
	}
}

void ACLMBasePawn::GetHealthAttributes_Implementation(float& OutCurrentHealth, float& OutMaxHealth)
{
	OutCurrentHealth = CurrentHealth;
	OutMaxHealth = MaxHealth;
}

void ACLMBasePawn::PerformRagdoll()
{
    CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (SkeletalMesh)
    {
		// 设置碰撞预设为 "Ragdoll"，这是一个专门为布娃娃设计的预设
        SkeletalMesh->SetCollisionProfileName(TEXT("Ragdoll"));
		// 启用物理模拟
        SkeletalMesh->SetSimulatePhysics(true);
		// 如果有需要，可以给一个初始的冲击力，让死亡效果更戏剧化
		SkeletalMesh->AddImpulse(FVector(-3000.f, 0, 3000.f), NAME_None, true);
    }
}

void ACLMBasePawn::Multicast_HandleDeath_Implementation()
{
	OnDeath();
}

// 这个RPC会在所有客户端上执行
void ACLMBasePawn::Multicast_PlayMontage_Implementation(UAnimMontage* MontageToPlay, float PlayRate)
{
    if (MontageToPlay)
    {
	    UE_LOG(LogTemp, Warning, TEXT("[%s] Multicast_PlayMontage playing montage: %s at rate: %.2f"), *GetName(), *MontageToPlay->GetName(), PlayRate);
    }
    else
    {
	    UE_LOG(LogTemp, Error, TEXT("[%s] Multicast_PlayMontage was called with a NULL Montage!"), *GetName());
		return;
    }

	if (SkeletalMesh && SkeletalMesh->GetAnimInstance())
	{
		SkeletalMesh->GetAnimInstance()->Montage_Play(MontageToPlay, PlayRate);
	}
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[%s] Failed to play montage, SkeletalMesh or AnimInstance is NULL."), *GetName());
    }
}

bool ACLMBasePawn::IsDead_Implementation()
{
	return bIsDead;
}

void ACLMBasePawn::EndHitStun()
{
	bIsInHitStun = false;

	// 硬直结束后，是时候做AI决策了！
	// 我们可以直接调用UpdateAI，让它决定是反击还是做别的事情
	UpdateAI();
}

void ACLMBasePawn::ClearState()
{

	//UE_LOG(LogTemp, Warning, TEXT("[%s]aaaaaaaaaaaaaaaaaaaaa ClearState"), *GetName());

    if (!HasAuthority()) return;
	
	const EUnitState PreviousState = CurrentState;
    CurrentState = EUnitState::IdlePawn;
    CurrentAttackTarget = nullptr;
    CommandedAttackTarget = nullptr;
    TargetResourceNode = nullptr;
	CarriedResourceEffectClass = nullptr;
    //ResourceDropOffBuilding = nullptr;

    GetWorld()->GetTimerManager().ClearTimer(AttackTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(GatheringTimerHandle);

	ResetAttackState();

    //清空“携带”的资源
	//------------------- 
    //在变为空闲时，设置速度为“闲逛速度”
	SetMaxSpeed(WanderSpeed);
	// 停止动画蒙太奇
	

    AAIController* PawnAIController = Cast<AAIController>(GetController());
    if (PawnAIController)
    {
        PawnAIController->StopMovement();
		//PawnAIController->ClearFocus(EAIFocusPriority::Gameplay);	
    }
	Multicast_StopAllMontages();
    // 检查Pawn之前是否正在执行一个非空闲的任务
    // 并且它现在确实要进入空闲状态了。
	/*
    if (PreviousState != EUnitState::IdlePawn && WanderBehaviorTree)
    {
        if (PawnAIController)
        {
            UE_LOG(LogTemp, Log, TEXT("[%s] Task finished. Restarting Wander Behavior Tree."), *GetName());
            // 确保先停止任何残留的逻辑（虽然通常在指令开始时已经停了）
            if(PawnAIController->GetBrainComponent())
            {
                PawnAIController->GetBrainComponent()->StopLogic("Clearing state to restart BT");
            }
            // 重新启动“闲逛”行为树
            PawnAIController->RunBehaviorTree(WanderBehaviorTree);
        }
    }
	*/
}

void ACLMBasePawn::Server_CommandGather_Implementation(AInteractableBase* ResourceToGather)
{
	/*
    if (!ResourceToGather) return;

    ClearState(); // 开始新任务前，清空旧状态

    TargetResourceNode = ResourceToGather;
    CurrentState = EUnitState::MovingToGather;
	UE_LOG(LogTemp, Warning, TEXT("[%s] Received Gather command. State: MovingToGather."), *GetName());


    AAIController* PawnAIController = Cast<AAIController>(GetController());
    if (PawnAIController)
    {
        //PawnAIController->MoveToActor(ResourceToGather, AttackRange * 0.8f);
		PawnAIController->MoveToActor(ResourceToGather, AttackRange);
    }
	*/
    if (!HasAuthority() || !ResourceToGather) return;

	SetMaxSpeed(CommandedSpeed);

    ClearState();

    TargetResourceNode = ResourceToGather;
    CurrentState = EUnitState::MovingToGather; // 只设置状态，移动交给UpdateAI
	//RequestPathTo(ResourceToGather); 
	UE_LOG(LogTemp, Warning, TEXT("[%s] Received Gather command. State: MovingToGather."), *GetName());


}

// 采集计时器完成后的回调
void ACLMBasePawn::OnGatheringComplete()
{
    if (!HasAuthority())
    {
        return;
    }

    AInteractableBase* ResourceNode = TargetResourceNode.Get();
    if (ResourceNode == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("[%s] OnGatheringComplete called, but TargetResourceNode is NULL! Clearing state."), *GetName());
        ClearState();
        return;
    }


	TSubclassOf<UGameplayEffect> ResourceEffectClass = ResourceNode->GetGrantedResourceEffect();
    // 从资源点获取资源
    const int32 GatheredAmount = ResourceNode->BeGathered(10);
	UE_LOG(LogTemp, Log, TEXT("[%s] Gathering complete. Gathered amount: %d"), *GetName(), GatheredAmount); // 假设一次采集10个

	if (GatheredAmount <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] Gathered nothing (Amount <= 0). Resource might be depleted. Clearing state."), *GetName());
		RestoreAutonomousAI();
        ClearState();
        return;
    }
		
    // 从资源节点获取它应该给予的GE，并“暂存”到我们的Pawn身上。
    //CarriedResourceEffectClass = ResourceNode->GetGrantedResourceEffect();
	if(GatheredAmount > 0 && ResourceEffectClass)
	{
		ATopDown_PlayerController* PlayerController = Cast<ATopDown_PlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));

		if (PlayerController)
		{
			const int32 PlayerFactionID = IFactionInterface::Execute_GetFaction(PlayerController);
			const int32 PawnFactionID = IFactionInterface::Execute_GetFaction(this);
			
			if (PawnFactionID == PlayerFactionID)
			{

			}
			else
			{
				UE_LOG(LogTemp, Log, TEXT("Enemy Pawn (Faction %d) gathered resources. Player (Faction %d) is unaffected."), PawnFactionID, PlayerFactionID);
			}
		}

		// 1. 获取玩家状态 (PlayerState)
		if (APlayerState* OwningPlayerState = PlayerController->PlayerState)
		{
			// 2. 获取能力系统组件 (ASC)
			if (UAbilitySystemComponent* ASC = OwningPlayerState->FindComponentByClass<UAbilitySystemComponent>())
			{
				// 3. 创建效果上下文和规格
				FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
				ContextHandle.AddSourceObject(this);
				FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(ResourceEffectClass, 1.0f, ContextHandle);

				// 4. 应用GE，直接增加玩家的资源
				if (SpecHandle.IsValid())
				{
					ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
					const int32 MyFactionID = IFactionInterface::Execute_GetFaction(this);
					UE_LOG(LogTemp, Warning, TEXT("Player Faction %d gathered resources!"), MyFactionID);
				}
			}
		}
        // 采集完一次后，再次检查目标资源是否还存在 (BeGathered可能会销毁它)
        if (TargetResourceNode.IsValid())
        {
            // 资源还在，那么就继续采集！
            UE_LOG(LogTemp, Log, TEXT("[%s] Resource still available. Starting next gather cycle."), *GetName());
            
            // 再次播放同一个采集动画，形成循环
            UAnimMontage* MontageToPlay = TargetResourceNode->GetGatheringMontage();
            if (MontageToPlay)
            {
                Multicast_PlayMontage(MontageToPlay, 1.5f);
            }
            else
            {
                // 如果没有动画，就直接清空状态，避免无限循环
                ClearState();
            }
        }
        else
        {
            // 资源在本次采集中被耗尽了
            UE_LOG(LogTemp, Warning, TEXT("[%s] Resource depleted. Clearing state."), *GetName());
			RestoreAutonomousAI();
            ClearState();
        }
	}
	else
	{
        // 如果没采集到或资源没设置效果，同样变为空闲
        if(GatheredAmount <= 0) UE_LOG(LogTemp, Warning, TEXT("[%s] Gathered nothing. Resource might be depleted."), *GetName());
        if(!ResourceEffectClass) UE_LOG(LogTemp, Error, TEXT("[%s] Resource has no 'Granted Resource Effect' set!"), *GetName());
		RestoreAutonomousAI();
        ClearState();		
	}

		/*   --------如果需要角色返回房子就使用这些代码,否则不用--------
		// 寻找最近的房子 (假设房子Actor带有 "TownHall" 标签)
		AActor* NearestBuilding = FindNearestActorWithTag(FName("TownHall"), FactionID);

		if (NearestBuilding)
		{
			UE_LOG(LogTemp, Warning, TEXT("[%s] Found nearest building: %s. Setting state to ReturningResource."), *GetName(), *NearestBuilding->GetName());
			
			// 1. 获取建筑的边界框信息，这能告诉我们建筑有多大
			FVector BuildingOrigin;
			FVector BuildingBoxExtent;
			NearestBuilding->GetActorBounds(false, BuildingOrigin, BuildingBoxExtent);

			// 2. 定义一个搜索半径，通常是建筑的对角线长度的一半左右，确保在建筑旁边
			const float SearchRadius = BuildingBoxExtent.Size();

			// 3. 获取导航系统
			UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
			FNavLocation RandomReachablePoint;
			
			bool bFoundPoint = false;
			if(NavSys)
			{
				bFoundPoint = NavSys->GetRandomReachablePointInRadius(BuildingOrigin, SearchRadius, RandomReachablePoint);
			}

			// 5. 将找到的随机点，或者如果没找到就用建筑的原始位置，作为我们的最终目标
			const FVector FinalDestination = bFoundPoint ? RandomReachablePoint.Location : BuildingOrigin;	

			ResourceDropOffBuilding = NearestBuilding;
			MoveTargerLocation = FinalDestination;
			CurrentState = EUnitState::ReturningResource; // 设置返回状态
			//RequestPathTo(NearestBuilding);

			// **重要**：在这里我们不再直接调用MoveToActor。
			// UpdateAI的下一个Tick会检测到新状态，并自动处理移动。
			AAIController* PawnAIController = Cast<AAIController>(GetController());
			if(PawnAIController)
			{
				PawnAIController->MoveToLocation(MoveTargerLocation);
				//PawnAIController->MoveToActor(ResourceDropOffBuilding.Get());
			}

		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[%s] Could not find any actor with tag 'TownHall' to return resources to!"), *GetName());
			ClearState(); // 找不到房子，原地待命
		}
		*/
}

// 寻找最近Actor的辅助函数
AActor* ACLMBasePawn::FindNearestActorWithTag(FName Tag, int32 RequiredFactionID) const
{
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), Tag, FoundActors);

    if (FoundActors.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("FindNearestActorWithTag: No actors found with tag '%s'"), *Tag.ToString());
        return nullptr;
    }
    
    AActor* NearestActor = nullptr;
    float MinDistanceSq = TNumericLimits<float>::Max();

    for (AActor* Actor : FoundActors)
    {
        if (!Actor) continue;
        // 1. 检查这个Actor是否实现了阵营接口
        if (Actor->GetClass()->ImplementsInterface(UFactionInterface::StaticClass()))
        {
            // 2. 获取它的阵营ID
            const int32 ActorFactionID = IFactionInterface::Execute_GetFaction(Actor);
            
            // 3. 如果它的阵营和我们需要的阵营不匹配，就跳过这个Actor
            if (ActorFactionID != RequiredFactionID)
            {
                continue; // 跳到下一次循环
            }
        }
        else
        {
            // 如果一个带 "TownHall" 标签的建筑没有实现阵营接口，我们也应该跳过它，以保证逻辑严谨
            continue;
        }
        const float DistanceSq = FVector::DistSquared(GetActorLocation(), Actor->GetActorLocation());
        if (DistanceSq < MinDistanceSq)
        {
            MinDistanceSq = DistanceSq;
            NearestActor = Actor;
        }
    }
    
    if(NearestActor)
    {
        UE_LOG(LogTemp, Log, TEXT("FindNearestActorWithTag: Nearest actor for Faction %d with tag '%s' is '%s'"), RequiredFactionID, *Tag.ToString(), *NearestActor->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("FindNearestActorWithTag: No actor with tag '%s' and Faction %d was found."), *Tag.ToString(), RequiredFactionID);
    }
    return NearestActor;
}

void ACLMBasePawn::OnGatherHit()
{
    // 这个函数由服务器权威地执行，以防止客户端作弊
    if (!HasAuthority())
    {
        return;
    }

    // 检查状态是否正确，防止在非采集状态下被意外调用
    if (CurrentState != EUnitState::Gathering)
    {
        return;
    }
    
    // 清除旧的3秒计时器，因为我们现在由动画驱动
    GetWorld()->GetTimerManager().ClearTimer(GatheringTimerHandle);

    // --- 调用我们之前已经写好的采集完成逻辑 ---
    // 这会让代码复用，非常整洁
    OnGatheringComplete();
}


void ACLMBasePawn::Multicast_StopAllMontages_Implementation()
{
	if (SkeletalMesh)
	{
		if (UAnimInstance* AnimInstance = SkeletalMesh->GetAnimInstance())
		{
			// Montage_Stop 会停止当前所有正在播放的蒙太奇
			// 0.25f 的淡出时间（Blend Out Time）可以让动画停止得更平滑，而不是瞬间卡住
			AnimInstance->Montage_Stop(0.25f);
		}
	}
}

// === AI采集指令的实现 ===
void ACLMBasePawn::AI_CommandGather(AInteractableBase* ResourceToGather)
{
	// 这个函数只在服务器上有效
	if (!HasAuthority()) return;

	// 直接调用我们已经写好的服务器函数，完美复用代码！
	Server_CommandGather_Implementation(ResourceToGather);
}

// === 检查是否空闲的函数实现 ===
bool ACLMBasePawn::IsIdle() const
{
	// 如果Pawn的当前状态是IdlePawn，那么它就是空闲的
	return CurrentState == EUnitState::IdlePawn;
}

void ACLMBasePawn::SetMaxSpeed(float NewSpeed)
{
	// 这个函数应该在服务器上被调用，因为移动是服务器权威的
	if (!HasAuthority()) return;

	// 检查移动组件是否存在
	if (FloatPawnMovement)
	{
		FloatPawnMovement->MaxSpeed = NewSpeed;
	}
	// 如果改用了ACharacter，这里的代码会是：
	/*
	if (UCharacterMovementComponent* CharMoveComp = GetCharacterMovement())
	{
		CharMoveComp->MaxWalkSpeed = NewSpeed;
	}
	*/
}

void ACLMBasePawn::StopAutonomousAI()
{
    if (!HasAuthority()) return;

    AAIController* AIController = Cast<AAIController>(GetController());
    if (AIController && AIController->GetBrainComponent())
    {
        // 只有当行为树正在运行时才停止它
        if(AIController->GetBrainComponent()->IsRunning())
        {
            UE_LOG(LogTemp, Log, TEXT("[%s] Autonomous AI stopped by player command."), *GetName());
            AIController->GetBrainComponent()->StopLogic("Player command override");
        }
    }
}

void ACLMBasePawn::RestoreAutonomousAI()
{
    if (!HasAuthority()) return;
    
    if (IsIdle())
    {
        AAIController* AIController = Cast<AAIController>(GetController());
        // 我们不再检查Pawn身上的WanderBehaviorTree
        if (AIController)
        {
			if(AIController->GetClass()->ImplementsInterface(UUnitAIInterface::StaticClass()))
			{
				UBehaviorTree* BehaviorTreeToRun = IUnitAIInterface::Execute_GetDefaultBehaviorTree(AIController);

				if(BehaviorTreeToRun)
				{
					AIController->RunBehaviorTree(BehaviorTreeToRun);
					UE_LOG(LogTemp, Warning, TEXT("[%s] Autonomous AI Restarted by running BT from interface."), *GetName());
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("[%s] AIController implements UnitAIInterface, but GetDefaultBehaviorTree() returned NULL!"), *GetName());
				}				
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("[%s] AIController '%s' does NOT implement UnitAIInterface!"), *GetName(), *AIController->GetName());
			}
        }
    }
}