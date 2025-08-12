// Fill out your copyright notice in the Description page of Project Settings.

#include "ResourceGenerator.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/BoxComponent.h"
#include "InteractableBase.h"

// Sets default values
AResourceGenerator::AResourceGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AResourceGenerator::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
    {
        this->GenerateResources();
    }
}

// Called every frame
void AResourceGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AResourceGenerator::GenerateResources()
{
    if (ResourceClasses.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("ResourceGenerator: No ResourceClasses specified."));
        return;
    }

    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!NavSys)
    {
        UE_LOG(LogTemp, Error, TEXT("ResourceGenerator: Failed to get NavigationSystemV1."));
        return;
    }

    const FVector GeneratorLocation = GetActorLocation();
    TArray<FVector> SpawnedLocations; // 用于记录已经生成了资源的位置

    for (int32 i = 0; i < NumberOfResourcesToSpawn; ++i)
    {
        FNavLocation RandomPoint;
        bool bFoundValidLocation = false;
        int32 Attempts = 0;
        FVector GroundLocation;
        

        // 尝试寻找一个有效的位置（最多尝试20次，防止死循环）
        while (!bFoundValidLocation && Attempts < 20)
        {
            Attempts++;
            // 1. 在导航网格上找到一个随机的 XY 点
            if (NavSys->GetRandomReachablePointInRadius(GeneratorLocation, GenerationRadius, RandomPoint))
            {
                // 从找到的点向上发射射线来确定地面高度 ===

                // 2. 定义射线的起点和终点
                // 起点在找到的随机点正上方1000个单位
                const FVector TraceStart = RandomPoint.Location + FVector(0, 0, 1000.0f);
                // 终点在找到的随机点正下方1000个单位
                const FVector TraceEnd = RandomPoint.Location - FVector(0, 0, 1000.0f);

                FHitResult HitResult;
                // 3. 执行单条射线检测。我们只关心 WorldStatic 类型的物体（比如地面）
                // 为了调试，我们可以让射线在编辑器中可见
                const bool bHit = UKismetSystemLibrary::LineTraceSingle(
                    GetWorld(),
                    TraceStart,
                    TraceEnd,
                    ETraceTypeQuery::TraceTypeQuery1, // 通常是 Visibility 通道
                    false, // bTraceComplex
                    TArray<AActor*>(), // ActorsToIgnore
                    EDrawDebugTrace::ForDuration, // DrawDebugType, 方便调试
                    HitResult,
                    true // bIgnoreSelf
                );

                // 4. 如果射线命中了某个东西
                if (bHit)
                {
                    // 我们将最终生成位置设置为射线命中点的位置
                    
                    GroundLocation = HitResult.Location;
                    bFoundValidLocation = true;

                    // 5. 检查这个新找到的点是否离其他已生成的资源太近
                    for (const FVector& SpawnedLoc : SpawnedLocations)
                    {
                        if (FVector::DistSquared2D(GroundLocation, SpawnedLoc) < FMath::Square(MinDistanceBetweenResources))                       
                        {
                            bFoundValidLocation = false; // 距离太近，位置无效，重新找
                            break;
                        }
                    }
                }
            }
        }
        
        // 如果成功找到了一个有效的位置
        if (bFoundValidLocation)
        {
            // 1. 从资源类型列表中随机选择一个资源
            
            const int32 RandomResourceIndex = FMath::RandRange(0, ResourceClasses.Num() - 1);
            TSubclassOf<AInteractableBase> ResourceToSpawn = ResourceClasses[RandomResourceIndex];

            if (ResourceToSpawn)
            {
                FVector FinalSpawnLocation = GroundLocation;
                // 1. 获取要生成的那个资源蓝图的 "默认对象 (CDO)"
                AInteractableBase* DefaultResourceObject = ResourceToSpawn->GetDefaultObject<AInteractableBase>();
                if (DefaultResourceObject)
                {
                    if (UBoxComponent* BoxComp = DefaultResourceObject->GetBoxComponent())
                    {
                        //获取Box的范围 (BoxExtent)，这是在缩放为1.0时的半高
                        const FVector UnscaledBoxExtent = BoxComp->GetUnscaledBoxExtent();
                        //获取你在编辑器里设置的相对缩放值
                        const FVector RelativeScale = BoxComp->GetRelativeScale3D();
                        //计算出最终的、经过缩放后的半高
                        const float ScaledHalfHeight = UnscaledBoxExtent.Z * RelativeScale.Z;
                        //高度偏移就是这个缩放后的半高
                        const float HeightOffset = ScaledHalfHeight;
                        //FVector BoxCenter, BoxExtent;
                        //float SphereRadius;
                        //UKismetSystemLibrary::GetComponentBounds(MeshComponent, BoxCenter, BoxExtent, SphereRadius);
                       
                        // 3. 计算高度偏移量。
                        //对于原点在底部的模型，这个偏移量就是Mesh边界框的Z轴范围(MeshBoxExtent.Z)。
                        // Case 1: 原点在中心, BoxCenter.Z ≈ 0, Offset ≈ BoxExtent.Z
                        // Case 2: 原点在底部, BoxCenter.Z ≈ BoxExtent.Z, Offset ≈ 0
                        //const float HeightOffset = BoxExtent.Z - BoxCenter.Z +300.f;
                        //const float HeightOffset = BoxExtent.Z + 20.0f;
                        //const float HeightOffset = UKismetMathLibrary::Abs(BoxExtent.Z);

                        // 4. 将我们之前探测到的地面位置，加上这个精确的高度偏移
                        FinalSpawnLocation.Z += HeightOffset;
                        UE_LOG(LogTemp, Log, TEXT("Spawning %s: UnscaledZ=%.2f, ScaleZ=%.2f, FinalOffset=%.2f"),
                            *DefaultResourceObject->GetName(), UnscaledBoxExtent.Z, RelativeScale.Z, HeightOffset);
                    }
                }
                // 2. 设置生成参数
                FActorSpawnParameters SpawnParams;
                //SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
                //SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
                //SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
                SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;


                // 3. 在找到的随机点上生成资源
                //GetWorld()->SpawnActor<AInteractableBase>(ResourceToSpawn, RandomPoint.Location, FRotator::ZeroRotator, SpawnParams);
                GetWorld()->SpawnActor<AInteractableBase>(ResourceToSpawn, FinalSpawnLocation, FRotator::ZeroRotator, SpawnParams);
                
                // 4. 记录这个位置，用于后续的距离检查
                //SpawnedLocations.Add(RandomPoint.Location);
                SpawnedLocations.Add(GroundLocation);

            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("ResourceGenerator: Finished spawning. Spawned %d resources."), SpawnedLocations.Num());
}