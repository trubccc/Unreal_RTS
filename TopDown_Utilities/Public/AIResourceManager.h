// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Resource_AttributeSet.h"
#include "AIResourceManager.generated.h"

UCLASS()
class TOPDOWN_UTILITIES_API AAIResourceManager : public AActor
{
	GENERATED_BODY()
private:
	// Key是阵营ID (FactionID)，Value是对应的属性集
	UPROPERTY()
	TMap<int32, TObjectPtr<UResource_AttributeSet>> FactionResources;

	// 一个静态指针，用于快速访问单例
	static AAIResourceManager* ResourceManagerInstance;

public:	
	// Sets default values for this actor's properties
	AAIResourceManager();
	// 一个全局可访问的单例实例
	static AAIResourceManager* GetInstance(UWorld* World);

	// 为一个指定阵营添加资源
	void AddResource(int32 FactionID, TSubclassOf<UGameplayEffect> ResourceEffectClass);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
