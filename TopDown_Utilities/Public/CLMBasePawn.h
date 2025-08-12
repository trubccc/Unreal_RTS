// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "BPI_PlayerController.h"
#include "SelectableInterface.h"
#include "AIController.h"
#include "Components/SphereComponent.h"
#include "NavigableInterface.h"
#include "BasePawnInterface.h"
#include "CombatInterface.h"
#include "CommonENUMS.h"
#include "FactionInterface.h"
#include "PawnInfoInterface.h"
#include "UnitAIInterface.h"
#include "BPI_Villager.h" 
#include "InteractableBase.h"
#include "GameplayEffect.h" 
#include "JobData.h"
#include "BehaviorTree/BehaviorTree.h"
#include "CLMBasePawn.generated.h"

class AAIController;
class UBehaviorTree;
class ATopDown_PlayerController;
class UCapsuleComponent;
class USkeletalMeshComponent;
class UFloatingPawnMovement;

//class ACLMBaseBuilding;

UCLASS()
class TOPDOWN_UTILITIES_API ACLMBasePawn : public APawn, public ISelectableInterface, public INavigableInterface, public IBasePawnInterface, public IFactionInterface, public IBPI_PlayerController, public ICombatInterface, public IPawnInfoInterface, public IBPI_Villager, public IUnitAIInterface

{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACLMBasePawn();
	
	//Hover
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pawn, meta = (AllowPrivateAccess = "true"))
	bool IsEnemy = false;
    // --- 新增：战斗属性 ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float MaxHealth = 100.0f;

    // 使用 ReplicatedUsing 来绑定一个OnRep函数，当生命值在客户端更新时，可以触发UI更新等
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", ReplicatedUsing = OnRep_CurrentHealth)
	float CurrentHealth;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float AttackDamage = 10.0f;	
    
    //在蓝图中为这些变量指定你的攻击和受击动画蒙太奇资源
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> HitMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> DieMontage;	

	//攻击速度/频率相关的变量 ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float AttackRate = 1.5f; // 每1.5秒攻击一次

    // --- 新增：用于受击硬直的计时器和状态 ---
	FTimerHandle HitStunTimerHandle;
	bool bIsInHitStun;

    // --- 新增：硬直结束后调用的函数 ---
	void EndHitStun();	

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> GetResourceMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI") 
	UBlackboardData* MyBlackboardAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI") 
	UBehaviorTree* MyBehaviorTree;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI") 
	UBlackboardComponent* BlackboardComponent;
	
private:
	//Capsule Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Collision, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCapsuleComponent> CapsuleComponent;

	//Skeletal Mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Pawn, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> SkeletalMesh;

	//Float Pawn movement 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Pawn, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UFloatingPawnMovement> FloatPawnMovement;

	//Float Pawn movement 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Pawn, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> SelectedIndicator;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pawn, meta = (AllowPrivateAccess = "true"))
	EActorType PawnType = EActorType::Villager;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pawn, meta = (AllowPrivateAccess = "true"))
	int32 FactionID = 1;
	
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pawn, meta = (AllowPrivateAccess = "true"))
	//TArray<TSubclassOf<ACLMBaseBuilding>> BuildingOptions;

    // 用于追踪当前正在攻击的目标，这个可以由AI或玩家设置
	UPROPERTY()
	TWeakObjectPtr<AActor> CurrentAttackTarget;
    
    // 用于攻击间隔的计时器
	FTimerHandle AttackTimerHandle;	

	//目标动态计算的接触距离
	float CurrentEngageRange;
	//标记是否正在攻击
	UPROPERTY(Replicated)
	bool bIsAttacking;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pawn, meta = (AllowPrivateAccess = "true"), Replicated)
	bool bIsDead;
	
	//武器碰撞的球体组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> WeaponCollisionSphere;
	
	UFUNCTION()
	void OnWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
    
	// 用一个TArray来存储所有攻击过我们的敌人
	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> Attackers;
	//TArray<TObjectPtr<AActor>> Attackers;

    // 专门存储由玩家命令的攻击目标 
	UPROPERTY()
	TWeakObjectPtr<AActor> CommandedAttackTarget;
    //一个函数来处理被攻击时的逻辑 
	void HandleBeingAttacked(AActor* DamageCauser);
    
    //一个AI更新函数，用于在Tick中做决策
	void UpdateAI(); 

	void ResetAttackState();

	

protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void PossessedBy(AController* NewController) override;
	void Move();

	//Navigation
	FVector MoveTargerLocation = FVector::ZeroVector;	
	bool bMoving = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pawn, meta = (AllowPrivateAccess = "true"))
	float AcceptanceRadius = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float AttackRange = 100.0f; // 这是最终的攻击触发距离

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float GatherRange = 200.0f;

    // 一个更大的“准备攻击”的范围
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float EngagePreparationRange = 150.0f; 

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pawn", meta = (AllowPrivateAccess = "true"))
	float TurnSpeed = 60.f; // 增加转向速度，让转身更干脆


    // 网络和战斗逻辑函数
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_CurrentHealth();
    
    // 检查是否在攻击范围内并执行攻击
	void CheckAndPerformAttack();

	// 实际执行攻击逻辑（播放动画、造成伤害）
	//	void PerformAttack();

	// 停止攻击
	void StopAttacking();

    // 死亡处理
	void Die();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_HandleDeath();
	
    UFUNCTION(BlueprintImplementableEvent, Category = "Pawn")
    void OnDeath();

	// 指向我们的任务数据表
	UPROPERTY(EditDefaultsOnly, Category = "AI|Jobs")
	TObjectPtr<UDataTable> JobsDataTable;

	// 当前正在执行的任务数据
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Jobs")
	FJobData CurrentJobData;

	// 当前的任务Tag
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Jobs")
	FGameplayTag CurrentJobTag;

	// 用于显示工具的组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ToolMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Category = "State", meta = (AllowPrivateAccess = "true"))
    EUnitState CurrentState = EUnitState::IdlePawn;

    // 清理所有状态和目标，让Pawn变回空闲
    void ClearState();	

    // 采集的目标
    UPROPERTY()
    TWeakObjectPtr<AInteractableBase> TargetResourceNode;

    // 资源送回的目标建筑
    //UPROPERTY()
    //TWeakObjectPtr<AActor> ResourceDropOffBuilding;

    // 采集计时器
    FTimerHandle GatheringTimerHandle;
    
    // 采集动画
    UPROPERTY(EditDefaultsOnly, Category = "Animation")
    UAnimMontage* GatheringMontage;
    
	//UPROPERTY(EditDefaultsOnly, Category = "Animation")
    //UAnimMontage* DropOffMontage;

    // 采集完成时调用的函数
    void OnGatheringComplete();

    // 寻找最近的带有指定标签的Actor（我们将用它来找房子）
    AActor* FindNearestActorWithTag(FName Tag, int32 RequiredFactionID) const;

    // === 新增：手动寻路所需变量 ===
    
    // 存储计算出的路径点
    TArray<FVector> CurrentPathPoints;

    // 当前路径点的索引
    int32 CurrentPathIndex;

    // 当Pawn从树上采集后，这个变量就会被设置为“给予木头”的GE。
    TSubclassOf<UGameplayEffect> CarriedResourceEffectClass;

	// 用于在所有客户端上停止动画的RPC
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StopAllMontages();

	UPROPERTY(EditAnywhere, Category = "AI")
	UBehaviorTree* WanderBehaviorTree;

	// 定义两种不同的速度
	// 在Pawn的蓝图默认值中方便地调整这两个速度
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float WanderSpeed = 150.0f; // AI闲逛时的慢速

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float CommandedSpeed = 600.0f; // 执行玩家指令时的正常速度

public:	

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	UFUNCTION()
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
	void SelectedActorLocal(const bool IsSelected);

	void SelectedActor_Implementation(const bool Selected) override;

	virtual void MoveToLocation_Implementation(const FVector TargetLocation) override;

	EActorType GetActorType_Implementation() override;

	//TArray<TSubclassOf<ACLMBaseBuilding>> GetBuildingOptions_Implementation() override;

	void SetFaction_Implementation(int32 Newfaction) override;
	
	int32 GetFaction_Implementation() override;
	
	//virtual void OnHover_Implementation() override;

	void OnHover_Implementation() override;


	void ClearHover_Implementation() override;
	
	UFUNCTION()
	void OnCapsuleBeginCursorOver(UPrimitiveComponent* TouchedComponent);

	UFUNCTION()
	void OnCapsuleEndCursorOver(UPrimitiveComponent* TouchedComponent);

    // --- 实现CombatInterface的函数 ---
	virtual void CommandAttackTarget_Implementation(AActor* TargetToAttack) override;
	virtual void TakeDamage_Implementation(float DamageAmount, AActor* DamageCauser) override;

	virtual void PlayerCommandAttackTarget_Implementation(AActor* TargetToAttack) override;

	virtual bool IsDead_Implementation() override;
    // 当移动时，应该停止攻击
	//用MoveToLocation_Implementation
	

	// 用于在所有客户端播放动画的RPC，我们给它一个默认值1.0f，这样旧的、没有提供速率的调用就不会报错
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayMontage(UAnimMontage* MontageToPlay, float PlayRate = 1.0f);

	//响应动画结束
	void AttackAnimationFinished();
	
	void EnableWeaponCollision();
	void DisableWeaponCollision();

	void AnimationFinished(UAnimMontage* Montage);

	void DropOffAnimationFinished(UAnimMontage* Montage);

	virtual void GetHealthAttributes_Implementation(float& OutCurrentHealth, float& OutMaxHealth) override;

    // 由PlayerController调用，开始采集流程
    UFUNCTION(Server, Reliable)
    void Server_CommandGather(AInteractableBase* ResourceToGather);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void OnGatherHit();

	// 一个专门给AI行为树调用的函数
	// 我们把它设为 BlueprintCallable，这样蓝图（行为树任务）就可以调用它了。
	UFUNCTION(BlueprintCallable, Category = "AI")
	void AI_CommandGather(AInteractableBase* ResourceToGather);

	//一个让行为树检查Pawn是否空闲的函数
	UFUNCTION(BlueprintPure, Category = "AI")
	bool IsIdle() const;

	//可在蓝图中调用的速度设置函数
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SetMaxSpeed(float NewSpeed);

	// 当玩家下达指令时，调用这个函数来停止自主AI
    UFUNCTION(BlueprintCallable, Category = "AI")
    void StopAutonomousAI();

	// 当玩家重新下达指令时，调用这个函数来恢复自主AI
    UFUNCTION(BlueprintCallable, Category = "AI")
    void RestoreAutonomousAI();

	UFUNCTION(BlueprintCallable, Category="Pawn")
    void PerformRagdoll();
};
