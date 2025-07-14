// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CommonENUMS.h"
#include "FactionInterface.h"
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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActorsSelected, const TArray<AActor*>&, SelectedActors);
UCLASS()
class TOPDOWN_UTILITIES_API ATopDown_PlayerController : public APlayerController, public IFactionInterface, public IBPI_PlayerController
{
	GENERATED_BODY()
	
public:
	ATopDown_PlayerController();
	UPROPERTY(EditDefaultsOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultInputMappingContext;



private:
	//Select Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> SelectAction;

	UPROPERTY(BlueprintAssignable, Category = "Delegates")
	FOnActorsSelected OnActorsSelected;
	
	//Command Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> CommandAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Faction, meta = (AllowPrivateAccess = "true"))
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

	
protected:

	virtual void BeginPlay() override;
	
	virtual void SetupInputComponent() override;

	void Select(const FInputActionValue& Value);

	void CommandSelectedActor(const FInputActionValue& Value);

	//Actor Rect Selection
	void SelectStart(const FInputActionValue& Value);
	void SelectOnGoing(const FInputActionValue& Value);
	void SelectEnd(const FInputActionValue& Value);

	

	void SelectMultipleActors();
	//-----
	
	void SetFaction_Implementation(int32 NewFactionID) override;

	int32 GetFaction_Implementation() override;

	virtual void OnHover_Implementation(bool bIsEnemy) override;
	virtual void ClearHover_Implementation() override;


};
