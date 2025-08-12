// Fill out your copyright notice in the Description page of Project Settings.


#include "TopDown_PlayerController.h"
#include "TopDown_PlayerState.h" 
#include "InputMappingContext.h"
#include "Net/UnrealNetwork.h"
#include "ActorData.h"
#include "ControlPawn.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "TopDown_HUD.h"
#include "CLMBasePawn.h"
#include "CombatInterface.h"
#include "CLMBaseBuilding.h"
#include "InteractableBase.h"      
#include "ResourceInterface.h"     
#include "AbilitySystemComponent.h" 
#include "GameplayTagContainer.h"  
#include "Abilities/GameplayAbilityTypes.h"



ATopDown_PlayerController::ATopDown_PlayerController()
{
    PrimaryActorTick.bCanEverTick = true;

    bShowMouseCursor = true;
    bEnableMouseOverEvents = true;
    bEnableClickEvents = true;

    // 初始化聚焦索引
    FocusIndex = 0;
    bIsFocus = false;
    TargetActor = nullptr;

	bIsPlacingBuilding = false;
	GhostBuilding = nullptr;    
}

void ATopDown_PlayerController::BeginPlay()
{
    Super::BeginPlay();
    TopDownHUD = Cast<ATopDown_HUD>(GetHUD());
}

void ATopDown_PlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 检查是否正在聚焦，并且目标Actor是否有效
    if(bIsFocus && IsValid(TargetActor))    
    {
        AControlPawn* PlayerPawn = GetPawn<AControlPawn>();
        if(PlayerPawn)
        {
            // 获取Pawn和Target的当前位置
            const FVector PawnLocation = PlayerPawn->GetActorLocation();
            const FVector TargetLocation = TargetActor->GetActorLocation();

            const FVector NewPawnLocation(TargetLocation.X, TargetLocation.Y, PawnLocation.Z);

            PlayerPawn->SetActorLocation(NewPawnLocation);
        }
    }

    //目标Actor失效了（比如被摧毁），则停止聚焦
    else if(bIsFocus)
    {
        BreakCameraFollow();
    }

	if (bIsPlacingBuilding && GhostBuilding)
	{
		// 更新幽灵建筑的位置
		FHitResult HitResult;
		GetHitResultUnderCursor(ECC_Visibility, false, HitResult);
		if (HitResult.bBlockingHit)
		{
			GhostBuilding->SetActorLocation(HitResult.Location);
			// 调用建筑自己的检查函数（我们等下会修改它）
			GhostBuilding->CheckPlaceValidity();
		}
	}    
}

void ATopDown_PlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if(!DefaultInputMappingContext)
    {
        return;
    }

    UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());

    if(Subsystem)
    {
        Subsystem->AddMappingContext(DefaultInputMappingContext, 0);
        UE_LOG(LogTemp, Display, TEXT("Input Mapping Context"));
    }

    if(UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
        //Bind Select function to select input action
        EnhancedInputComponent->BindAction(SelectAction, ETriggerEvent::Completed, this, &ATopDown_PlayerController::Select);

        EnhancedInputComponent->BindAction(SelectAction, ETriggerEvent::Started, this, &ATopDown_PlayerController::SelectStart);
        EnhancedInputComponent->BindAction(SelectAction, ETriggerEvent::Triggered, this, &ATopDown_PlayerController::SelectOnGoing);
        EnhancedInputComponent->BindAction(SelectAction, ETriggerEvent::Completed, this, &ATopDown_PlayerController::SelectEnd);

        //Bind Command function 
        EnhancedInputComponent->BindAction(CommandAction, ETriggerEvent::Completed, this, &ATopDown_PlayerController::CommandSelectedActor);

        //Bind Focus function 
        EnhancedInputComponent->BindAction(FocusAction, ETriggerEvent::Completed, this, &ATopDown_PlayerController::FocusOnSelectedActor);

		EnhancedInputComponent->BindAction(SelectAction, ETriggerEvent::Triggered, this, &ATopDown_PlayerController::HandlePlaceBuilding);
 		EnhancedInputComponent->BindAction(CommandAction, ETriggerEvent::Triggered, this, &ATopDown_PlayerController::HandleCancelPlacement);               
    }
}

void ATopDown_PlayerController::Select(const FInputActionValue& Value)
{
    UE_LOG(LogTemp, Display, TEXT("Select Action"));

    FHitResult HitResult;
    GetHitResultUnderCursor(ECollisionChannel::ECC_Camera, false, HitResult);

    //Deselect previous Pawn
    if(SelectedActor)
    {
        if(SelectedActor->GetClass()->ImplementsInterface(USelectableInterface::StaticClass()))
        {
            ISelectableInterface::Execute_SelectedActor(SelectedActor, false);
        }

        // 3. 检查这个被取消选择的单位是否是我们的Pawn
        if (ACLMBasePawn* PawnToDeselect = Cast<ACLMBasePawn>(SelectedActor))
        {
            // 4. 如果是，就调用RPC通知服务器恢复它的自主行为
            //Server_RestorePawnAutonomy(PawnToDeselect);
            UE_LOG(LogTemp, Warning, TEXT("Selected Actor is aaaabbbbbbbbb Pawn"));
            PawnToDeselect->RestoreAutonomousAI();
        }
        else
        {
            UE_LOG(LogTemp, Display, TEXT("Selected Actor is not a Pawn"));

        }       
    }

    SelectedActor = HitResult.GetActor();
    FocusIndex = 0;
    

    if(SelectedActor)
    {
        //UE_LOG(LogTemp, Display, TEXT("Selected Actor: %s"), *SelectedActor->GetName());
        
        //select new Pawn
        if(SelectedActor->GetClass()->ImplementsInterface(USelectableInterface::StaticClass()))
        {
            ISelectableInterface::Execute_SelectedActor(SelectedActor, true);
        }
        /*
        if (ACLMBasePawn* CommandedPawn = Cast<ACLMBasePawn>(SelectedActor))
        {
            // 暂停AI
            CommandedPawn->StopAutonomousAI();
        }
        */        
    }
}



void ATopDown_PlayerController::CommandSelectedActor(const FInputActionValue& Value)
{
    FHitResult HitResult;   
    GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, false, HitResult);

    if(!HitResult.bBlockingHit || SelectedActors.Num() == 0)
    {
        return;
    }

    // 1. 遍历所有当前选中的单位
    for (AActor* CommandSelectedActor : SelectedActors)
    {
        if (ACLMBasePawn* CommandPawn = Cast<ACLMBasePawn>(CommandSelectedActor))
        {
            // 2. 直接在客户端调用Pawn身上的服务器RPC
            // 这个指令会以最高优先级立刻发送给服务器
            CommandPawn->StopAutonomousAI();
        }
    }
    AActor* HitActor = HitResult.GetActor();
    ACLMBasePawn* HitPawn = Cast<ACLMBasePawn>(HitActor);    

    Server_CommandSelectedActors(SelectedActors, HitResult.Location, HitActor);

    /*
    if(SelectedActors.Num() > 0)
    {
        if(HitPawn && HitPawn->IsEnemy)
        {
            Server_CommandSelectedActors(SelectedActors, FVector::ZeroVector, HitPawn);

        }
        else
        {
            Server_CommandSelectedActors(SelectedActors, HitResult.Location, nullptr);
        }
        /*  
        int i = SelectedActors.Num() / 2;
        for(AActor* SomeActor : SelectedActors)
        {
            if(SomeActor)
            {
                if(SomeActor->GetClass()->ImplementsInterface(UFactionInterface::StaticClass()))
                {
                    int ActorFaction = IFactionInterface::Execute_GetFaction(SomeActor);
                    if(FactionID != ActorFaction)
                    {
                        continue;
                    }
                }


                if(SomeActor->GetClass()->ImplementsInterface(UNavigableInterface::StaticClass()))
                {
                    INavigableInterface::Execute_MoveToLocation(SomeActor, HitResult.Location + FVector(0, 100 * i, 0));
                    i++;
                }
            }
        }         
    }
    */
    /*
    else
    {
        
        if(SelectedActor->GetClass()->ImplementsInterface(UFactionInterface::StaticClass()))
        {   
            int ActorFaction = IFactionInterface::Execute_GetFaction(SelectedActor);
            if(FactionID != ActorFaction)
            {
                return;
            }
        }

        if(SelectedActor->GetClass()->ImplementsInterface(UNavigableInterface::StaticClass()))
        {

            INavigableInterface::Execute_MoveToLocation(SelectedActor, HitResult.Location);        
        }
    }*/
}


void ATopDown_PlayerController::SelectStart(const FInputActionValue& Value)
{   
    float MouseX, MouseY;
    GetMousePosition(MouseX, MouseY);
    SelectStartPosition = FVector2D(MouseX, MouseY);
    //UE_LOG(LogTemp, Display, TEXT("Selection Start"));
}

void ATopDown_PlayerController::SelectOnGoing(const FInputActionValue& Value)
{
    float MouseX, MouseY;
    GetMousePosition(MouseX, MouseY);
    SelectionSize = FVector2D(MouseX - SelectStartPosition.X, MouseY - SelectStartPosition.Y);

    //UE_LOG(LogTemp, Display, TEXT("Selection OnGoing"));

    if(TopDownHUD)
    {
        TopDownHUD->ShowSelectionRect(SelectStartPosition, SelectionSize);
    }
}

void ATopDown_PlayerController::SelectEnd(const FInputActionValue& Value)
{
    UE_LOG(LogTemp, Display, TEXT("Selection End"));
    if(TopDownHUD)  
    {
        TopDownHUD->HideSelectionRect();
        FTimerHandle TimerHandleSelectMultipleActors;
        GetWorld()->GetTimerManager().SetTimer(TimerHandleSelectMultipleActors, this, &ATopDown_PlayerController::SelectMultipleActors, 0.05f, false);
    }
    OnActorsSelected.Broadcast(SelectedActors);
}

void ATopDown_PlayerController::FocusOnSelectedActor(const FInputActionValue &Value)
{
    UE_LOG(LogTemp, Warning, TEXT("F key pressed. FocusOnSelectedActor called."));
    if(SelectedActors.Num() == 0) 
    {
        BreakCameraFollow();
        return;
    }

    if(!SelectedActors.IsValidIndex(FocusIndex))
    {
        //UE_LOG(LogTemp, Warning, TEXT("FocusIndex %d is invalid. Resetting to 0."), FocusIndex);
        FocusIndex = 0;
        if(!SelectedActors.IsValidIndex(FocusIndex))
        {
            //UE_LOG(LogTemp, Error, TEXT("FocusIndex is still invalid after reset. Aborting."));
            BreakCameraFollow();
            return;
        }
    } 

    AActor* PotentialTarget = SelectedActors[FocusIndex];
    //TargetActor = SelectedActors[FocusIndex];
    if (!IsValid(PotentialTarget))
    {

        // 如果目标无效，就从列表里移除，然后重试
        SelectedActors.RemoveAt(FocusIndex);
        if (SelectedActors.Num() > 0)
        {
            // 确保索引不会越界
            FocusIndex = FocusIndex % SelectedActors.Num(); 
            FocusOnSelectedActor(Value); // 重新调用
        }
        else
        {
            BreakCameraFollow();
        }
        return;                
    }

    // 如果我们正在聚焦，并且目标就是即将要聚焦的同一个角色，那么就取消聚焦（实现toggle效果）
    if(bIsFocus && TargetActor == PotentialTarget)
    {
        UE_LOG(LogTemp, Warning, TEXT("Toggling OFF focus for"));
        BreakCameraFollow();
        // 保持FocusIndex不变，这样下次按F还会是这个单位
        return;

    }

    // 设置新的目标
    TargetActor = PotentialTarget;
    AControlPawn* PlayerPawn = GetPawn<AControlPawn>();

    if(PlayerPawn && TargetActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("Focusing on %s"), *TargetActor->GetName());
        // 计算并存储相机与目标的相对偏移量，只在开始聚焦时计算一次
        
        // 1. 获取当前Pawn和目标的位置
        const FVector PawnLocation = PlayerPawn->GetActorLocation();
        const FVector TargetLocation = TargetActor->GetActorLocation();

        const FVector NewPawnLocation(TargetLocation.X, TargetLocation.Y, PawnLocation.Z);
        PlayerPawn->SetActorLocation(NewPawnLocation, false, nullptr, ETeleportType::TeleportPhysics);

        // 启动聚焦状态，Tick函数将处理后续的移动
        bIsFocus = true;   
        //PlayerPawn->SetActorLocation(TargetActor->GetActorLocation(), false, nullptr, ETeleportType::TeleportPhysics);     
    }
    else
    {
        if (!TargetActor) UE_LOG(LogTemp, Error, TEXT("TargetActor is null!"));
        if (!PlayerPawn) UE_LOG(LogTemp, Error, TEXT("PlayerPawn is null!"));
    }
    //更新索引，为下一次按F键做准备
    FocusIndex = (FocusIndex + 1) % SelectedActors.Num();
    UE_LOG(LogTemp, Warning, TEXT("Next FocusIndex will be: %d"), FocusIndex);
}

void ATopDown_PlayerController::SelectMultipleActors()
{
    
    if(TopDownHUD)
    {
        //Deselect old
        for(AActor* SomeActor : SelectedActors)
        {
            if(SomeActor)
            {
                if(SomeActor->GetClass()->ImplementsInterface(USelectableInterface::StaticClass()))
                {
                    ISelectableInterface::Execute_SelectedActor(SomeActor, false);
                }
                // 3. 检查这个单位是否是我们的Pawn
                if (ACLMBasePawn* PawnToDeselect = Cast<ACLMBasePawn>(SomeActor))
                {
                    // 4. 如果是，就调用RPC通知服务器恢复它的自主行为
                    //Server_RestorePawnAutonomy(PawnToDeselect);
                    UE_LOG(LogTemp, Warning, TEXT("Selected Actor is a Pawn"));
                    PawnToDeselect->RestoreAutonomousAI();
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("Selected Actor is not aaa Pawn"));

                }                                   
            }
        }         

        SelectedActors.Empty();
        FocusIndex = 0;
        
        //Select new
        TArray<AActor*> AllSelectedActors = TopDownHUD->GetSelectedActors();
        
        if(AllSelectedActors.Num() == 1)
        {
            AActor* SomeActor = AllSelectedActors[0];
            if(SomeActor->GetClass()->ImplementsInterface(USelectableInterface::StaticClass()))
            {
                ISelectableInterface::Execute_SelectedActor(SomeActor, true);
                SelectedActors.AddUnique(SelectedActor);
            }
            /*
            // 然后告诉他们可以恢复自主行为了
            if (ACLMBasePawn* CommandedPawn = Cast<ACLMBasePawn>(SomeActor))
            {
                //停止自动AI
                CommandedPawn->StopAutonomousAI();
            }              
            */
        }
        else 
        {
            for(AActor* SomeActor : AllSelectedActors)
            {
                if(SomeActor)
                {
                    if(SomeActor->GetClass()->ImplementsInterface(UFactionInterface::StaticClass()))
                    {
                        int32 ActorFaction = IFactionInterface::Execute_GetFaction(SomeActor);
                        if(FactionID != ActorFaction)
                        { 
                            continue;
                        }
                    }                
                    if(SomeActor)
                    {
                        if(SomeActor->GetClass()->ImplementsInterface(USelectableInterface::StaticClass()))
                        {
                            ISelectableInterface::Execute_SelectedActor(SomeActor, true);
                            SelectedActors.AddUnique(SomeActor);
                        }
                        /*
                        if (ACLMBasePawn* CommandedPawn = Cast<ACLMBasePawn>(SomeActor))
                        {
                            //停止自动AI
                            CommandedPawn->StopAutonomousAI();
                        }
                        */
                    }
                }
            }
        }
        OnActorsSelected.Broadcast(SelectedActors);
    }
}

void ATopDown_PlayerController::SetFaction_Implementation(int32 NewFactionID)
{
    FactionID = NewFactionID;
}

int32 ATopDown_PlayerController::GetFaction_Implementation()
{
    return FactionID;
}
/*
void ATopDown_PlayerController::OnHover_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("OnHover_Implementation"));
	CurrentMouseCursor = IsEnemy? EMouseCursor::Crosshairs : EMouseCursor::Hand;
}

void ATopDown_PlayerController::ClearHover_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("ClearHover_Implementation"));
    CurrentMouseCursor = EMouseCursor::Default;
}
*/

bool ATopDown_PlayerController::Server_CommandSelectedActors_Validate(const TArray<AActor*>& ActorsToCommand, const FVector& TargetLocation, AActor* AttackTarget)
{
    return true; 
}

void ATopDown_PlayerController::Server_CommandSelectedActors_Implementation(const TArray<AActor*> &ActorsToCommand, const FVector &TargetLocation, AActor* AttackTarget)
{
    // --- 新增采集指令逻辑 ---
    AInteractableBase* InteractableTarget = Cast<AInteractableBase>(AttackTarget);
    if (InteractableTarget)
    {
        // 指令目标是可交互资源，发出采集指令
        for (AActor* CommandedActor : ActorsToCommand)
        {
            ACLMBasePawn* CommandedPawn = Cast<ACLMBasePawn>(CommandedActor);
            if (CommandedPawn)
            {
                // 调用Pawn身上新的采集函数
                CommandedPawn ->Server_CommandGather(InteractableTarget);
            }
        }
        return; // 处理完毕，直接返回
    }   

    // 首先，判断点击的目标是否是一个可以攻击的敌人
    bool bIsAttackCommand = false;
    if (AttackTarget && AttackTarget->GetClass()->ImplementsInterface(UCombatInterface::StaticClass()))
    {
        // 进一步检查它是否是敌对阵营
        // 这个检查应该在服务器上做，防止作弊
        if (AttackTarget->GetClass()->ImplementsInterface(UFactionInterface::StaticClass()))
        {
            const int32 TargetFaction = IFactionInterface::Execute_GetFaction(AttackTarget);
            // 假设玩家阵营ID是FactionID，并且不等于目标阵营ID
            if (TargetFaction != FactionID) 
            {
                bIsAttackCommand = true;
            }
        }
    }

    // 如果是攻击指令
    if (bIsAttackCommand)
    {
        for (AActor* Attacker : ActorsToCommand)
        {
            // 确保攻击者也实现了战斗接口
            if (Attacker && Attacker->GetClass()->ImplementsInterface(UCombatInterface::StaticClass()))
            {
                // 命令单位攻击目标
                //ICombatInterface::Execute_CommandAttackTarget(Attacker, AttackTarget);
                ICombatInterface::Execute_PlayerCommandAttackTarget(Attacker, AttackTarget);
            }
        }
    }
    else
    {
        // 因为这是在服务器上运行的，所以它对单位的移动是权威的
        int i = (ActorsToCommand.Num() / 2);
        for (AActor* SomeActor : ActorsToCommand)
        {
            if (SomeActor)
            {
                // 在服务器上再次验证这个单位是否属于这个玩家
                if (SomeActor->GetClass()->ImplementsInterface(UFactionInterface::StaticClass()))
                {
                    int ActorFaction = IFactionInterface::Execute_GetFaction(SomeActor);
                    if (FactionID != ActorFaction)
                    {
                        continue; // 防止玩家控制不属于自己的单位
                    }
                }

                if (SomeActor->GetClass()->ImplementsInterface(UNavigableInterface::StaticClass()))
                {
                    const FVector IndividualTargetLocation = TargetLocation + FVector(0, 100 * i, 0);
                    INavigableInterface::Execute_MoveToLocation(SomeActor, IndividualTargetLocation);
                    i++;
                }
            }
        }      
    } 
}


void ATopDown_PlayerController::BreakCameraFollow()
{
    if(bIsFocus)
    {
        UE_LOG(LogTemp, Display, TEXT("Camera Follow Broken."));
        bIsFocus = false;
        TargetActor = nullptr;
    }
}

void ATopDown_PlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // 如果有需要复制的变量，像这样添加它们：
    // DOREPLIFETIME(ATopDown_PlayerController, YourReplicatedVariable);
}
/*
void ATopDown_PlayerController::StartPlacingBuilding(TSubclassOf<ACLMBaseBuilding> BuildingClass)
{
	if (!BuildingClass) return;

	// 生成一个建筑实例
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	ACLMBaseBuilding* GhostBuilding = GetWorld()->SpawnActor<ACLMBaseBuilding>(BuildingClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	if (GhostBuilding)
	{
		// 调用建筑自己的函数，让它进入“放置模式”
		GhostBuilding->EnablePlaceMode();
	}
}
*/
// 进入放置模式
void ATopDown_PlayerController::EnterBuildingPlacementMode(TSubclassOf<ACLMBaseBuilding> BuildingClass)
{
	if (BuildingClass == nullptr) return;

	bIsPlacingBuilding = true;
	// 生成一个“幽灵”建筑，它只在本地存在，用于预览
	GhostBuilding = GetWorld()->SpawnActor<ACLMBaseBuilding>(BuildingClass);
	if (GhostBuilding)
	{
		// 禁用它的碰撞，因为它只是个视觉预览
		GhostBuilding->SetActorEnableCollision(false);
	}
}

TArray<TSubclassOf<ACLMBaseBuilding>> ATopDown_PlayerController::GetBuildingOptions_Implementation()
{
    return BuildingOptions;
}

// 退出放置模式
void ATopDown_PlayerController::ExitBuildingPlacementMode()
{
	bIsPlacingBuilding = false;
	if (GhostBuilding)
	{
		GhostBuilding->Destroy();
		GhostBuilding = nullptr;
	}
}

void ATopDown_PlayerController::HandlePlaceBuilding()
{
	// 只在放置模式下响应
	if (bIsPlacingBuilding && GhostBuilding)
	{
        UE_LOG(LogTemp, Log, TEXT("Player trying to place building..."));
		if (GhostBuilding->CanBePlaced()) // 我们会给建筑添加这个函数
		{
            UE_LOG(LogTemp, Warning, TEXT("Placement check PASSED. Sending request to server."));
			// 如果可以放置，就通过RPC请求服务器生成一个真的
			Server_PlaceBuilding(GhostBuilding->GetClass(), GhostBuilding->GetActorTransform());
			// 成功放置后，退出放置模式
			ExitBuildingPlacementMode();
		}
		else
		{
			// 播放一个“无法放置”的音效或UI提示
			UE_LOG(LogTemp, Warning, TEXT("Cannot place building here!"));
		}
	}
    // 如果不在放置模式，这里可以留空，或者调用你之前的单位选择逻辑
    // SelectStart(...); // 你可能需要调整这里的逻辑，避免冲突
}

void ATopDown_PlayerController::HandleCancelPlacement()
{
	if (bIsPlacingBuilding)
	{
        // 通知服务器我们取消了
        Server_CancelBuildingPlacement();
        
        ExitBuildingPlacementMode();
	}
    // 如果不在放置模式，这里就是正常的右键指令
    // CommandSelectedActor(...);
}

// 服务器RPC的实现
void ATopDown_PlayerController::Server_PlaceBuilding_Implementation(TSubclassOf<ACLMBaseBuilding> ClassToSpawn, FTransform SpawnTransform)
{
	if (ClassToSpawn)
	{
		ACLMBaseBuilding* NewBuilding = GetWorld()->SpawnActor<ACLMBaseBuilding>(ClassToSpawn, SpawnTransform);
		if (NewBuilding)
		{
			// 在这里，在服务器上，调用生成单位的逻辑！
			NewBuilding->SpawnInitialPawns();
			// 还可以设置阵营等
			// NewBuilding->SetFaction_Implementation(this->GetFaction_Implementation());
		}
	}
}

void ATopDown_PlayerController::Server_RestorePawnAutonomy_Implementation(ACLMBasePawn* PawnToRestore)
{
    if (PawnToRestore)
    {
        PawnToRestore->RestoreAutonomousAI();
    }
}

void ATopDown_PlayerController::RequestEnterBuildingMode(TSubclassOf<ACLMBaseBuilding> BuildingClass)
{
    // 直接向服务器发送请求
    Server_RequestEnterBuildingMode(BuildingClass);
}

// 服务器端的请求处理
void ATopDown_PlayerController::Server_RequestEnterBuildingMode_Implementation(TSubclassOf<ACLMBaseBuilding> BuildingClass)
{
    if (BuildingClass == nullptr) return;

    FBuildCastData BuildCostData;
    UE_LOG(LogTemp, Warning, TEXT("SERVER: Received RequestEnterBuildingMode for %s."), *BuildingClass->GetName());

    if (ACLMBaseBuilding::GetBuildCost(BuildingClass, BuildCostData))
    {
        UE_LOG(LogTemp, Warning, TEXT("SERVER: Successfully got build cost."));

        ATopDown_PlayerState* PS = GetPlayerState<ATopDown_PlayerState>();
        if (PS)
        {
            UE_LOG(LogTemp, Warning, TEXT("SERVER: PlayerState is valid. Checking cost..."));
            if (PS->CanBuildCast(BuildCostData.BuildCast))
            {
                UE_LOG(LogTemp, Warning, TEXT("SERVER: Can afford! Spending resources..."));
                PS->SpendBuildCost(BuildCostData.BuildCast);
                Client_EnterBuildingPlacementMode(BuildingClass);
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("SERVER: Cannot afford! Request denied."));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("SERVER: FAILED to get ATopDown_PlayerState! Check GameMode settings!"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("SERVER: FAILED to get build cost! Check Data Table and ActorType enum."));
    }
}

// 客户端的视觉表现函数 (这个函数内容就是你旧的EnterBuildingPlacementMode)
void ATopDown_PlayerController::Client_EnterBuildingPlacementMode(TSubclassOf<ACLMBaseBuilding> BuildingClass)
{
    if (BuildingClass == nullptr) return;
    
    // 如果已经在放置模式，先取消旧的
    if(bIsPlacingBuilding)
    {
        HandleCancelPlacement();
    }

	bIsPlacingBuilding = true;
	GhostBuilding = GetWorld()->SpawnActor<ACLMBaseBuilding>(BuildingClass);
	if (GhostBuilding)
	{
		GhostBuilding->SetActorEnableCollision(false);
        // 记录下我们正在建造的建筑类型，取消时需要用到
        BuildingClassToPlace = BuildingClass; 
	}
}


// 修改HandleCancelPlacement，让它能通知服务器



// 新的取消RPC的实现
void ATopDown_PlayerController::Server_CancelBuildingPlacement_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("SERVER: Received CancelBuildingPlacement."));
    if (BuildingClassToPlace)
    {
        FBuildCastData BuildCostData;
        if (ACLMBaseBuilding::GetBuildCost(BuildingClassToPlace, BuildCostData))
        {
            ATopDown_PlayerState* PS = GetPlayerState<ATopDown_PlayerState>();
            if (PS)
            {
                PS->RevertBuildCast(BuildCostData.BuildCast);
                UE_LOG(LogTemp, Warning, TEXT("SERVER: Resources reverted for %s."), *BuildingClassToPlace->GetName());
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("SERVER: FAILED to get ATopDown_PlayerState during cancellation!"));
            }
        }
        BuildingClassToPlace = nullptr;
    }
}