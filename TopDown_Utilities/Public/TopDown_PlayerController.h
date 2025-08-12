// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CommonENUMS.h"
#include "FactionInterface.h"
#include "BasePawnInterface.h"
#include "AbilitySystemInterface.h"
#include "BPI_PlayerController.h"
#include "TopDown_PlayerController.generated.h"

/**
 * 
 */

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class ACLMBasePawn;
class ATopDown_HUD;
class ACLMBaseBuilding;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActorsSelected, const TArray<AActor*>&, SelectedActors);
UCLASS()
class TOPDOWN_UTILITIES_API ATopDown_PlayerController : public APlayerController, public IFactionInterface, public IBPI_PlayerController, public IBasePawnInterface
{
	GENERATED_BODY()
	
public:
	ATopDown_PlayerController();
	UPROPERTY(EditDefaultsOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultInputMappingContext;
	void BreakCameraFollow();
	
    // 在这里定义可建造列表 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Building")
	TArray<TSubclassOf<ACLMBaseBuilding>> BuildingOptions;
    
    // 一个函数，用于开始建造流程
	//UFUNCTION(BlueprintCallable, Category = "Building")
	//void StartPlacingBuilding(TSubclassOf<ACLMBaseBuilding> BuildingClass);

	UFUNCTION(BlueprintCallable, Category = "Building")
	void EnterBuildingPlacementMode(TSubclassOf<ACLMBaseBuilding> BuildingClass);

	TArray<TSubclassOf<ACLMBaseBuilding>> GetBuildingOptions_Implementation() override;

	UFUNCTION(BlueprintCallable, Category = "Building")
    void RequestEnterBuildingMode(TSubclassOf<ACLMBaseBuilding> BuildingClass);

private:
	//Select Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> SelectAction;

	UPROPERTY(BlueprintAssignable, Category = "Delegates")
	FOnActorsSelected OnActorsSelected;
	
	//Command Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> CommandAction;

	//FocusAction Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> FocusAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Faction, meta = (AllowPrivateAccess = "true"), Replicated)
	int32 FactionID = 1;	
	
	//Select BasePawn Actor
	
	UPROPERTY()
	TObjectPtr<AActor> SelectedActor;
	

	UPROPERTY()
	TObjectPtr<ATopDown_HUD> TopDownHUD;

	//Actor Rect Selection
	FVector2D SelectStartPosition;
	FVector2D SelectionSize;

	TArray<AActor*> SelectedActors;

	//用于在选定单位之间循环的索引
	UPROPERTY()
	int32 FocusIndex;

	bool bIsFocus;

	UPROPERTY()
	TObjectPtr<AActor> TargetActor;

	bool bIsPlacingBuilding;

	UPROPERTY()
	TObjectPtr<ACLMBaseBuilding> GhostBuilding; // 指向我们正在放置的“幽灵建筑”

    // 处理放置和取消的输入函数
	void HandlePlaceBuilding();
	void HandleCancelPlacement();

    // 用于启动/停止放置模式的内部函数
	void ExitBuildingPlacementMode();

    // 用于网络同步的RPC 
	UFUNCTION(Server, Reliable)
	void Server_PlaceBuilding(TSubclassOf<ACLMBaseBuilding> ClassToSpawn, FTransform SpawnTransform);

protected:
	

	virtual void BeginPlay() override;
	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupInputComponent() override;

	void Select(const FInputActionValue& Value);

	void CommandSelectedActor(const FInputActionValue& Value);

	//Actor Rect Selection
	void SelectStart(const FInputActionValue& Value);
	void SelectOnGoing(const FInputActionValue& Value);
	void SelectEnd(const FInputActionValue& Value);

	void FocusOnSelectedActor(const FInputActionValue& Value);

	void SelectMultipleActors();
	//-----
	
	void SetFaction_Implementation(int32 NewFactionID) override;

	int32 GetFaction_Implementation() override;

	//virtual void OnHover_Implementation() override;
	//virtual void ClearHover_Implementation() override;

	//Replication
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_CommandSelectedActors(const TArray<AActor*>& ActorsToCommand, const FVector& TargetLocation, AActor* AttackTarget);

    //void Server_CommandSelectedActors_Implementation(const TArray<AActor*>& ActorsToCommand, const FVector& TargetLocation, AActor* AttackTarget);
    
    UFUNCTION(Server, Reliable)
    void Server_RestorePawnAutonomy(ACLMBasePawn* PawnToRestore);

	UPROPERTY()
    TSubclassOf<ACLMBaseBuilding> BuildingClassToPlace;
	
	UFUNCTION(Server, Reliable)
    void Server_RequestEnterBuildingMode(TSubclassOf<ACLMBaseBuilding> BuildingClass);

    // 用于客户端显示幽灵建筑的函数
    void Client_EnterBuildingPlacementMode(TSubclassOf<ACLMBaseBuilding> BuildingClass);

    // 新的RPC，用于通知服务器取消建造
    UFUNCTION(Server, Reliable)
    void Server_CancelBuildingPlacement();

};
