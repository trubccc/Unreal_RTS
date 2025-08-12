// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ResourceGenerator.generated.h"


class AInteractableBase;
UCLASS()
class TOPDOWN_UTILITIES_API AResourceGenerator : public AActor
{
	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
	AResourceGenerator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	// 1. 可以生成的资源种类的列表
	// 我们可以在蓝图中填充这个数组，比如把 BP_Tree 和 BP_Stone 加进来
	UPROPERTY(EditAnywhere, Category = "Resource Generation")
	TArray<TSubclassOf<AInteractableBase>> ResourceClasses;

	// 2. 在指定区域内生成的资源总数量
	UPROPERTY(EditAnywhere, Category = "Resource Generation", meta = (ClampMin = "1"))
	int32 NumberOfResourcesToSpawn = 50;

	// 3. 以生成器为中心，在多大的半径范围内生成资源
	UPROPERTY(EditAnywhere, Category = "Resource Generation", meta = (ClampMin = "100.0"))
	float GenerationRadius = 3000.0f;
    
    // 4. 生成的资源之间至少要保持多大的距离，防止它们重叠在一起
    UPROPERTY(EditAnywhere, Category = "Resource Generation", meta = (ClampMin = "10.0"))
    float MinDistanceBetweenResources = 200.0f;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void GenerateResources();

};
