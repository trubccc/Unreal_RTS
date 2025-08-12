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
	// Key����ӪID (FactionID)��Value�Ƕ�Ӧ�����Լ�
	UPROPERTY()
	TMap<int32, TObjectPtr<UResource_AttributeSet>> FactionResources;

	// һ����ָ̬�룬���ڿ��ٷ��ʵ���
	static AAIResourceManager* ResourceManagerInstance;

public:	
	// Sets default values for this actor's properties
	AAIResourceManager();
	// һ��ȫ�ֿɷ��ʵĵ���ʵ��
	static AAIResourceManager* GetInstance(UWorld* World);

	// Ϊһ��ָ����Ӫ�����Դ
	void AddResource(int32 FactionID, TSubclassOf<UGameplayEffect> ResourceEffectClass);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
