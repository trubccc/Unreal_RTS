// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "BPI_PlayerController.h"
#include "SelectableInterface.h"
#include "NavigableInterface.h"
#include "BasePawnInterface.h"
#include "CommonENUMS.h"
#include "FactionInterface.h"
#include "CLMBasePawn.generated.h"

class UCapsuleComponent;
class USkeletalMeshComponent;
class UFloatingPawnMovement;

UCLASS()
class TOPDOWN_UTILITIES_API ACLMBasePawn : public APawn, public ISelectableInterface, public INavigableInterface, public IBasePawnInterface, public IFactionInterface, public IBPI_PlayerController
{
	GENERATED_BODY()

	public:
	// Sets default values for this pawn's properties
	ACLMBasePawn();

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
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pawn, meta = (AllowPrivateAccess = "true"))
	TArray<TSubclassOf<ACLMBaseBuilding>> BuildingOptions;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void Move();

	//Navigation
	FVector MoveTargerLocation = FVector::ZeroVector;
	bool bMoving = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pawn, meta = (AllowPrivateAccess = "true"))
	float AcceptanceRadius = 50.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pawn, meta = (AllowPrivateAccess = "true"))
	float TurnSpeed = 5.f;

	//Hover
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pawn, meta = (AllowPrivateAccess = "true"))
	bool IsEnemy = false;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	UFUNCTION()
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
	void SelectedActorLocal(const bool IsSelected);

	void SelectedActor_Implementation(const bool Selected) override;

	void MoveToLocation_Implementation(const FVector TargetLocation) override;

	EActorType GetActorType_Implementation() override;

	TArray<TSubclassOf<ACLMBaseBuilding>> GetBuildingOptions_Implementation() override;

	void SetFaction_Implementation(int32 Newfaction) override;
	int32 GetFaction_Implementation() override;

	void OnHover_Implementation(bool bIsEnemy) override;

	void ClearHover_Implementation() override;
	
	UFUNCTION()
	void OnCapsuleBeginCursorOver(UPrimitiveComponent* TouchedComponent);

	UFUNCTION()
	void OnCapsuleEndCursorOver(UPrimitiveComponent* TouchedComponent);
};
