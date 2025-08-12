// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ActorData.h"
#include "CommonENUMS.h"
#include "BasePawnInterface.h"
#include "SelectableInterface.h"	
#include "FactionInterface.h"
#include "ResourceInterface.h"
#include "CLMBasePawn.h"
#include "CLMBaseBuilding.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
struct FInputActionValue;
class UInputAction;

UCLASS()
class TOPDOWN_UTILITIES_API ACLMBaseBuilding : public AActor, public ISelectableInterface, public IFactionInterface, public IBasePawnInterface, public IResourceInterface
{
	GENERATED_BODY()
	
private:

	//Mesh Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Building, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> StaticMesh;

	//Selected indicator
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Building, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> SelectedIndicator;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Faction, meta = (AllowPrivateAccess = "true"))
	int32 FactionID = 1;

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Building, meta = (AllowPrivateAccess = "true"))
	//TObjectPtr<UInputAction> PlaceAction;

	//UPROPERTY()
	//FTimerHandle PlaceBuildingTimerHandle;
	
	//Need to have this tag to be able to place the building
	UPROPERTY()	
	FName RequiredTag = "CanPlaceBuildings"; 

	UPROPERTY()	
	FVector BuildingExtends = FVector(500.f, 500.f, 500.f);  //Size of building extends, used for collision check

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Building, meta = (AllowPrivateAccess = "true"))
	EActorType ActorType = EActorType::House;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Building, meta = (AllowPrivateAccess = "true"))
	TArray<TSubclassOf<ACLMBaseBuilding>> BuildingOptions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Building, meta = (AllowPrivateAccess = "true"))
	UDataTable* DT_BuildCost;

public:	
	// Sets default values for this actor's properties
	ACLMBaseBuilding();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
    // 在蓝图中指定这个建筑能够生成的角色类型
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	TSubclassOf<ACLMBasePawn> PawnToSpawn;

    // 生成的数量
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	int32 NumberOfPawnsToSpawn = 2;

    // 在距离建筑多远的半径内随机寻找生成点
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	float SpawnRadius = 500.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UBoxComponent* PlacementCollision;

	void UpdatePlacementState(bool bNewCanBePlaced);

	// 新增一个函数，用于蓝图来实现变色逻辑
	UFUNCTION(BlueprintImplementableEvent, Category = "Building Placement")
	void OnPlacementValidityChanged(bool bCanBePlaced);

	// 这个变量是我们的“唯一数据源”
	UPROPERTY()
	bool bCanPlacing = false;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	//Implementing Selectable interface
	void SelectedActor_Implementation(const bool Selected) override;

	void SetFaction_Implementation(int32 NewFactionID) override;
	int32 GetFaction_Implementation() override;

	//Building placing mode
	//UFUNCTION(BlueprintCallable, Category = "Building")
	//void EnablePlaceMode();

	//Check Location validity for building
	void CheckPlaceValidity();

	bool CanBePlaced() const { return bCanPlacing; }
	//Place the building
	//void PlaceBuilding(const FInputActionValue& Value);

	//Cancel building 
	void CancelBuilding(const FInputActionValue& Value);

	//Revert cast
	//void RevertBuildingCast();


	UFUNCTION(BlueprintImplementableEvent, Category = "Building")
	void ToggleBuildingValidity(bool bValid);

	EActorType GetActorType_Implementation() override;

	TArray<TSubclassOf<ACLMBaseBuilding>> GetBuildingOptions_Implementation() override;

	void SpawnInitialPawns();

    // 实现资源接口
    void DepositResource_Implementation(EResourceType Type, int32 Amount) override;

	UFUNCTION(BlueprintPure, Category = "Building")
    static bool GetBuildCost(TSubclassOf<ACLMBaseBuilding> BuildingClass, FBuildCastData& OutCost);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	UTexture2D* Icon;

	UBoxComponent* GetPlacementCollision() const;

};
