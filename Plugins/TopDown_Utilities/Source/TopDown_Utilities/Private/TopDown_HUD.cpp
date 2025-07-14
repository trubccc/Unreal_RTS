// Fill out your copyright notice in the Description page of Project Settings.


#include "TopDown_HUD.h"
#include "CLMBasePawn.h"
#include "CLMBaseBuilding.h"


void ATopDown_HUD::DrawHUD()
{
    Super::DrawHUD();
    if(bDrawSelectionRect)
    {
        DrawRect(SelectionRectColor, SelectionRectStart.X, SelectionRectStart.Y, SelectionRectSize.X, SelectionRectSize.Y);
    }
    if(bSelectedActor)
    {
        SelectActorsInRect();
    }
}


void ATopDown_HUD::ShowSelectionRect(const FVector2D InSelectionRectStart, const FVector2D InSelectionRectSize)
{
    SelectionRectStart = InSelectionRectStart;
    SelectionRectSize = InSelectionRectSize;
    bDrawSelectionRect = true;
}

void ATopDown_HUD::HideSelectionRect()
{
    bDrawSelectionRect = false;
    bSelectedActor = true;
}

TArray<AActor*> ATopDown_HUD::GetSelectedActors()
{
    return SelectedActors;
}

void ATopDown_HUD::SelectActorsInRect()
{   
    SelectedActors.Empty();
    FVector2D FirstPoint = SelectionRectStart;
    FVector2D SecondPoint = SelectionRectStart + SelectionRectSize;

    //Select Pawr
    TArray<ACLMBasePawn*> SelectedPawn;
    GetActorsInSelectionRectangle<ACLMBasePawn>(FirstPoint, SecondPoint, SelectedPawn, false);

    if(SelectedPawn.Num() > 0)
    {
        bSelectedActor = false;
        for(ACLMBasePawn* Pawn : SelectedPawn)
        {
            if(Pawn)
            {
                SelectedActors.AddUnique(Pawn);
            }
            
        }
        return;
    }

    //If no pawn, select building
    TArray<ACLMBaseBuilding*> SelectedBuildings;
    GetActorsInSelectionRectangle<ACLMBaseBuilding>(FirstPoint, SecondPoint, SelectedBuildings, false);
    if(SelectedBuildings.Num() > 0)
    {
        bSelectedActor = false;
        for(ACLMBaseBuilding* Building : SelectedBuildings)
        {
            if(Building)
            {
                SelectedActors.AddUnique(Building);
            }
        }
        return;
    }

}