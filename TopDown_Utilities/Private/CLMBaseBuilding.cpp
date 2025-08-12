// Fill out your copyright notice in the Description page of Project Settings.


#include "CLMBaseBuilding.h"
#include "EnhancedInputComponent.h"
#include "ActorData.h"
#include "InputActionValue.h"
#include "TopDown_PlayerState.h"
#include "Components/BoxComponent.h"
#define ECC_Building ECC_GameTraceChannel3


// Sets default values
ACLMBaseBuilding::ACLMBaseBuilding()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PlacementCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("PlacementCollision"));
	SetRootComponent(PlacementCollision);

	PlacementCollision->SetCollisionObjectType(ECC_Building);
	PlacementCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	PlacementCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	PlacementCollision->SetCollisionResponseToChannel(ECC_Building, ECollisionResponse::ECR_Overlap);

	// e. 同时，让它能检测到与Pawn的重叠（用于防止在单位身上造建筑）
	PlacementCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	//Create MeshComponent
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	//SetRootComponent(StaticMesh);
	StaticMesh->SetupAttachment(RootComponent);
	StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//Create Selected Indicator
	SelectedIndicator = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SelectedIndicator"));
	SelectedIndicator->SetupAttachment(StaticMesh);
	SelectedIndicator->SetHiddenInGame(true);
	SelectedIndicator->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Tags.Add(FName("Building"));
    // 这会让Pawn知道这是一个可以返回资源的建筑（比如主城、仓库等）
    Tags.Add(FName("TownHall")); 	
}

// Called when the game starts or when spawned
void ACLMBaseBuilding::BeginPlay()
{
	Super::BeginPlay();
	
}

void ACLMBaseBuilding::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACLMBaseBuilding::SpawnInitialPawns()
{
	// 1. 安全检查：确保我们设置了要生成的Pawn类型
	if (!PawnToSpawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("PawnToSpawn is not set in building %s"), *GetName());
		return;
	}

	// 2. 获取建筑的变换信息
	const FVector BuildingLocation = GetActorLocation();
	const FRotator BuildingRotation = GetActorRotation();
	const FVector ForwardVector = GetActorForwardVector(); // 获取建筑的“前方”方向向量

	// 3. 定义Pawn生成的基础偏移位置。例如，在建筑前方150个单位处。
	const float SpawnOffsetDistance = 500.0f;
	// 定义两个Pawn之间的间隔
	const float Spacing = 100.0f;
	
	// 4. 循环生成指定数量的Pawn
	for (int32 i = 0; i < NumberOfPawnsToSpawn; ++i)
	{
		// 计算每个Pawn的横向偏移
		// 对于2个Pawn，第一个i=0，偏移-50；第二个i=1，偏移+50
		const float LateralOffset = (i - (NumberOfPawnsToSpawn - 1) / 2.0f) * Spacing;
		const FVector RightVector = GetActorRightVector(); // 获取建筑的“右方”方向向量
		const FVector UpVector = GetActorUpVector(); // 获取建筑的“上方”方向向量
		// 计算最终的生成位置
		// 公式：建筑位置 + 向前偏移 + 向侧方偏移
		const FVector SpawnLocation = BuildingLocation + ForwardVector * SpawnOffsetDistance + RightVector * LateralOffset + UpVector * 150.f;
		
		// 生成的Pawn朝向与建筑相同
		const FRotator SpawnRotation = BuildingRotation;

		// 设置Spawn参数
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		
		ACLMBasePawn* NewPawn = GetWorld()->SpawnActor<ACLMBasePawn>(PawnToSpawn, SpawnLocation, SpawnRotation, SpawnParams);

		if (NewPawn)
		{
			NewPawn->AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
			NewPawn->SpawnDefaultController();
			UE_LOG(LogTemp, Log, TEXT("Spawned a pawn at %s"), *SpawnLocation.ToString());
			// 为新生成的Pawn设置阵营
			if (NewPawn->GetClass()->ImplementsInterface(UFactionInterface::StaticClass()))
			{
				IFactionInterface::Execute_SetFaction(NewPawn, this->FactionID);
			}
		}
	}
}

void ACLMBaseBuilding::DepositResource_Implementation(EResourceType Type, int32 Amount)
{
	
}

// Called every frame

void ACLMBaseBuilding::SelectedActor_Implementation(const bool Selected)
{
	SelectedIndicator->SetHiddenInGame(!Selected);
}

void ACLMBaseBuilding::SetFaction_Implementation(int32 NewFactionID)
{
	FactionID = NewFactionID;
}

int32 ACLMBaseBuilding::GetFaction_Implementation()
{
	return FactionID;
}
/*
void ACLMBaseBuilding::EnablePlaceMode()
{
	UE_LOG(LogTemp, Display, TEXT("Enable Place Mode"));
	//Enable Inputs on	 this actor
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();

	if(PlayerController)
	{
		//Add Input Mapping Context
		UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerController->InputComponent);
		if(EnhancedInputComponent)
		{
			//Add Input Mapping Context
			EnhancedInputComponent->BindAction(PlaceAction, ETriggerEvent::Completed, this, &ACLMBaseBuilding::PlaceBuilding);

		}
	}

	GetWorld()->GetTimerManager().SetTimer(PlaceBuildingTimerHandle, this, &ACLMBaseBuilding::CheckPlaceValidity, 0.03f, true);
	this->SetActorEnableCollision(false);
}
*/
void ACLMBaseBuilding::CheckPlaceValidity()
{
	UE_LOG(LogTemp, Display, TEXT("Check Place Validity"));

	//UPrimitiveComponent* MyCollisionComponent = GetPlacementCollision();

	//ToggleBuildingValidity(bCanPlacing);

	//bCanPlacing = true;

	//Set building Location under cursor(follow)
	FHitResult HitResult;
	GetWorld()->GetFirstPlayerController()->GetHitResultUnderCursor(ECollisionChannel::ECC_WorldStatic, false, HitResult);
	if(!HitResult.bBlockingHit || (RequiredTag != NAME_None && !HitResult.GetActor()->ActorHasTag(RequiredTag)))
	{
		UE_LOG(LogTemp, Warning, TEXT("No Hit Result"));
		UpdatePlacementState(false);

		if (!HitResult.bBlockingHit)
		{
			UE_LOG(LogTemp, Warning, TEXT("Placement Check: Invalid ground (No Hit)."));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Placement Check: Invalid ground (Tag '%s' missing on '%s')."), *RequiredTag.ToString(), *HitResult.GetActor()->GetName());
		}
		//ToggleBuildingValidity(false);
		return;
	}
	//SetActorLocation(HitResult.Location);

	//PlacementCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	TArray<AActor*> OverlappingActors;
	PlacementCollision->GetOverlappingActors(OverlappingActors, ACLMBaseBuilding::StaticClass());

	//MyCollisionComponent->GetOverlappingActors(OverlappingActors);
	UE_LOG(LogTemp, Log, TEXT("Found %d overlapping actors of class ACLMBaseBuilding."), OverlappingActors.Num());

	// c. 遍历所有重叠的Actor
	for (AActor* OverlappedActor : OverlappingActors)
	{
		if (OverlappedActor && OverlappedActor != this)
		{
			// 如果碰到了任何一个其他的建筑，就立刻判定为“不可放置”
			UpdatePlacementState(false);
			UE_LOG(LogTemp, Warning, TEXT("Placement Check: Blocked by overlapping building '%s'."), *OverlappedActor->GetName());
			return;
		}
	}
	UpdatePlacementState(true);
	UE_LOG(LogTemp, Log, TEXT("Placement Check: Valid."));
	// 4. 如果代码能执行到这里，说明通过了所有检查
	// 标记为可以放置
	//ToggleBuildingValidity(bCanPlacing);
	//bCanPlacing = true;
	/*
	//Define parameters for box sweep
	FVector BoxHalfExtent = BuildingExtends / 2;
	FVector TraceStart = HitResult.Location + FVector(0.f, 0.f, BoxHalfExtent.Z);
	FVector TraceEnd = TraceStart + FVector::UpVector;

	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);
	CollisionParams.bTraceComplex = false;
	TArray<FHitResult> OutHits;

	//Perform a box sweep to check for collisions
	bool bHit = GetWorld()->SweepMultiByChannel(OutHits, TraceStart, TraceEnd, GetActorRotation().Quaternion(), ECC_Visibility, FCollisionShape::MakeBox(BoxHalfExtent), CollisionParams);

	for(const FHitResult& Hit : OutHits)
	{
		if(!(Hit.GetActor() != nullptr && Hit.GetActor()->ActorHasTag(RequiredTag)))
		{
			//UE_LOG(LogTemp, Warning, TEXT("Area blocked by: %s"), *Hit.GetActor()->GetName());
			return;
		}
	}

	bCanPlacing = true;
	*/
}
/*
void ACLMBaseBuilding::PlaceBuilding(const FInputActionValue& Value)
{
	if(!bCanPlacing)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot Place"));
		RevertBuildingCast();
		SetLifeSpan(0.1f);	//Destory if cannot place
		return;
	}
	
	UE_LOG(LogTemp, Display, TEXT("Place Building"));

	//clear timer handle
	GetWorld()->GetTimerManager().ClearTimer(PlaceBuildingTimerHandle);
	this->SetActorEnableCollision(true);
	if (HasAuthority()) 
	{
		SpawnInitialPawns();
	}
	
	DisableInput(GetWorld()->GetFirstPlayerController());
	
}
*/
void ACLMBaseBuilding::CancelBuilding(const FInputActionValue& Value)	
{
	UE_LOG(LogTemp, Display, TEXT("Cancel Building"));
}
/*
void ACLMBaseBuilding::RevertBuildingCast()
{
	if(!DT_BuildCost)
	{
		UE_LOG(LogTemp, Warning, TEXT("No Build Cost Data Table"));
		return;
	}
	
	FString ActorTypeStr = UEnum::GetValueAsString(ActorType);
	FString CleanNameStr;
	ActorTypeStr.Split(TEXT("::"), nullptr, &CleanNameStr);
	FBuildCastData* BuildCast_S = DT_BuildCost->FindRow<FBuildCastData>(FName(CleanNameStr), TEXT(""));

	if(BuildCast_S)
	{
		ATopDown_PlayerState* PlayerState = Cast<ATopDown_PlayerState>(GetWorld()->GetFirstPlayerController()->PlayerState);
		PlayerState->RevertBuildCast(BuildCast_S->BuildCast);	
	}
	
}
*/
EActorType ACLMBaseBuilding::GetActorType_Implementation()
{
    return ActorType;
}

TArray<TSubclassOf<ACLMBaseBuilding>> ACLMBaseBuilding::GetBuildingOptions_Implementation()
{
    return BuildingOptions;
}

bool ACLMBaseBuilding::GetBuildCost(TSubclassOf<ACLMBaseBuilding> BuildingClass, FBuildCastData& OutCost)
{
    if (BuildingClass == nullptr) return false;

    // 获取该建筑类的“默认对象 (CDO)”
    ACLMBaseBuilding* DefaultObject = BuildingClass->GetDefaultObject<ACLMBaseBuilding>();
    if (DefaultObject && DefaultObject->DT_BuildCost)
    {
        // 从默认对象的Data Table中查询花费
        FString ActorTypeStr = UEnum::GetValueAsString(DefaultObject->ActorType);
        FString CleanNameStr;
        ActorTypeStr.Split(TEXT("::"), nullptr, &CleanNameStr);
        FBuildCastData* FoundCost = DefaultObject->DT_BuildCost->FindRow<FBuildCastData>(FName(CleanNameStr), TEXT(""));
        
        if (FoundCost)
        {
            OutCost = *FoundCost;
            return true;
        }
    }
    return false;
}

UBoxComponent* ACLMBaseBuilding::GetPlacementCollision() const
{
	return PlacementCollision;
}

void ACLMBaseBuilding::UpdatePlacementState(bool bNewCanBePlaced)
{
	// 只有当状态真正发生改变时，我们才执行操作，避免不必要的刷新
	if (bCanPlacing != bNewCanBePlaced)
	{
		//更新数据状态
		bCanPlacing = bNewCanBePlaced;

		//调用蓝图事件，让蓝图去处理视觉变化（比如变色）
		OnPlacementValidityChanged(bCanPlacing);
	}
}