// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "TopDown_HUD.generated.h"

/**
 * 
 */
UCLASS()
class TOPDOWN_UTILITIES_API ATopDown_HUD : public AHUD
{
	GENERATED_BODY()
	
private:
	bool bDrawSelectionRect = false;
	bool bSelectedActor = false;
	FVector2D SelectionRectStart;
	FVector2D SelectionRectSize;
	FLinearColor SelectionRectColor = FLinearColor(1.f, 0.f, 0.f, .2f);

	TArray<AActor*> SelectedActors;
	void SelectActorsInRect();

protected:
	virtual void DrawHUD() override;

public:
	void ShowSelectionRect(const FVector2D InSelectionRectStart, const FVector2D InSelectionRectSize);
	void HideSelectionRect();

	TArray<AActor*> GetSelectedActors();
};
