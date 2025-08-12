// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractableBase.h"
#include "Net/UnrealNetwork.h"
// Sets default values
	AInteractableBase::AInteractableBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	// 1. 创建一个根组件
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));

    // 2. 创建用于“物理阻挡”的静态网格组件 (我们的“小碰撞体”)
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMesh->SetupAttachment(RootComponent);

    // - 它是一个动态物体，因为它可能会被销毁
    StaticMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    // - 它只阻挡Pawn，防止穿模
	StaticMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	StaticMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
    // - 启用它的碰撞
    StaticMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);


	// 3. 创建用于“鼠标交互”的BoxComponent (我们的“大碰撞体”)
	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
	BoxComponent->SetupAttachment(RootComponent); // 也附加到根

    // - 它是一个静态物体，用于场景查询
	BoxComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
    // - 它只响应鼠标点击，忽略其他一切
	BoxComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	BoxComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
    // - 启用它的碰撞
	BoxComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    // - **重要**: 将这个Box设置得比StaticMesh稍大一些，方便玩家点击
    BoxComponent->SetBoxExtent(FVector(100.f, 100.f, 100.f)); // 你可以在蓝图中根据模型大小调整

	// 4. 添加资源标签
	Tags.Add(FName("ResourceNode"));
}

void AInteractableBase::OnRep_ProgressionState()
{
	OnProgressionStateChanged(ProgressionState);
}

void AInteractableBase::BeginPlay()
{
	Super::BeginPlay();

	// 设置初始网格
	OnProgressionStateChanged(ProgressionState);

	// Cropout的放置时重叠销毁逻辑
	if (HasAuthority())
	{
		// 检查是否在放置模式
		if (!ActorHasTag(FName("PlacementMode")))
		{
			TArray<AActor*> OverlappingActors;
			BoxComponent->GetOverlappingActors(OverlappingActors, AInteractableBase::StaticClass());

			for (AActor* OverlappedActor : OverlappingActors)
			{
				if (OverlappedActor != this)
				{
					Destroy();
					return;
				}
			}
		}
	}	
}

// Called every frame
void AInteractableBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


// Called when the game starts or when spawned
void AInteractableBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AInteractableBase, ProgressionState);
}

int32 AInteractableBase::BeGathered(int32 AmountToTake)
{
    if (HasAuthority())
    {
        const int32 TakenAmount = FMath::Min(AmountToTake, TotalAmount);
        TotalAmount -= TakenAmount;
        if (TotalAmount <= 0)
        {
            Destroy(); // 资源耗尽后销毁
        }
        return TakenAmount;
    }
    return 0;
}



void AInteractableBase::Interact(AActor* InteractingActor)
{
	// 只有服务器能处理交互逻辑
	if (!HasAuthority()) return;

	// 播放摇摆特效（在所有客户端上）
	// 你需要一个NetMulticast RPC来触发蓝图事件
	// ...

	// 处理建造逻辑
	if (bRequiresBuild && ProgressionState < MeshProgressionList.Num() - 1)
	{
		ProgressionState++;
		OnProgressionStateChanged(ProgressionState);
		return; // 建造一次，然后返回
	}

	// 如果不是建造，那就是采集
	// 这里的逻辑可以移到采集能力(GA_GatherResource)中
	// 比如，采集能力在完成后，可以调用一个函数来减少这个资源的总量
	
}

FGameplayTag AInteractableBase::GetRequiredAbilityTag_Implementation() const
{
	return RequiredAbilityTag;
}

FGameplayTag AInteractableBase::GetResourceTypeTag()
{
    return ResourceTypeTag;
}

int32 AInteractableBase::GetHarvestAmount()
{
    return HarvestAmount;
}

TSubclassOf<UGameplayEffect> AInteractableBase::GetGrantedResourceEffect()
{
    return GrantedResourceEffect;
}

UAnimMontage* AInteractableBase::GetGatheringMontage()
{
    return GatheringMontage;
}
