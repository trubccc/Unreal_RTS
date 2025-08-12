// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ResourceInterface.h"
#include "GameFramework/Actor.h"
#include "CommonENUMS.h"
#include "GameplayEffect.h"
#include "Animation/AnimMontage.h" 
#include "Components/BoxComponent.h"
#include "InteractableBase.generated.h"


class UStaticMeshComponent;


UCLASS()
class TOPDOWN_UTILITIES_API AInteractableBase : public AActor, public IResourceInterface
{
	GENERATED_BODY()
private:
	//Box Component	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Collision, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> BoxComponent;

	//Mesh Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Interactable, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> StaticMesh;

public:	
	// Sets default values for this actor's properties
	AInteractableBase();
    // 资源类型
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource")
    EResourceType ResourceType;

    // 资源总量
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource", Replicated)
    int32 TotalAmount;

    // 每次采集量
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource")
    int32 AmountPerGather;

    // 采集此资源需要的时间
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource")
    float TimeToGather;

    // 被采集的函数，由单位调用
    UFUNCTION(BlueprintCallable, Category = "Resource")
    int32 BeGathered(int32 AmountToTake);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// 采集这个资源需要的能力Tag
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Resource|Abilities")
	FGameplayTag RequiredAbilityTag;

	// 这个资源是什么类型？
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Resource|Type")
	FGameplayTag ResourceTypeTag;

	// 每次采集能获得多少数量？
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Resource|Harvesting")
	int32 HarvestAmount = 10;

	// 建造/升级所需的网格列表 (0=地基, 1=建造中, 2=成品)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Resource|Progression")
	TArray<TObjectPtr<UStaticMesh>> MeshProgressionList;

	// 是否需要建造过程？
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Resource|Progression")
	bool bRequiresBuild = false;

	// 当前的建造/生命周期阶段
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_ProgressionState, Category = "Resource|Progression")
	int32 ProgressionState = 0;

	UFUNCTION()
	void OnRep_ProgressionState();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	// 在蓝图中实现的函数，用于更新视觉效果
	UFUNCTION(BlueprintImplementableEvent, Category = "Progression")
	void OnProgressionStateChanged(int32 NewState);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montage") 
	UAnimMontage* InteractMontage;

	// 我们可以直接在每个资源的蓝图（如BP_Tree, BP_Stone）中设置这个变量。
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource")
    TSubclassOf<UGameplayEffect> GrantedResourceEffect;

    // 定义这个资源节点专属的采集动画
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource")
    UAnimMontage* GatheringMontage;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void Interact(AActor* InteractingActor);
	
	// 播放摇摆特效的蓝图事件
	UFUNCTION(BlueprintImplementableEvent, Category = "Effects")
	void PlayWobbleEffect();

	// --- 实现 IResourceInterface ---
	virtual FGameplayTag GetRequiredAbilityTag_Implementation() const override;

	// --- Getters ---
	UFUNCTION(BlueprintCallable, Category="Resource")
	FGameplayTag GetResourceTypeTag();

	UFUNCTION(BlueprintCallable, Category="Resource")
	int32 GetHarvestAmount();

    // 添加一个公共的Getter函数，方便Pawn获取这个效果
	UFUNCTION(BlueprintCallable, Category="Resource")
    TSubclassOf<UGameplayEffect> GetGrantedResourceEffect();

    // 添加一个公共的Getter函数，方便Pawn获取这个动画
	UFUNCTION(BlueprintCallable, Category="Resource")
    UAnimMontage* GetGatheringMontage();

	//getter函数来获取StaticMesh
	UStaticMeshComponent* GetStaticMesh() { return StaticMesh; }

	UBoxComponent* GetBoxComponent() const { return BoxComponent; }

};
