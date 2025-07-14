// Fill out your copyright notice in the Description page of Project Settings.


#include "TopDown_PlayerController.h"
#include "InputMappingContext.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "TopDown_HUD.h"
#include "CLMBasePawn.h"

ATopDown_PlayerController::ATopDown_PlayerController()
{
    bShowMouseCursor = true;
    bEnableMouseOverEvents = true;
    bEnableClickEvents = true;
}

void ATopDown_PlayerController::BeginPlay()
{
    Super::BeginPlay();
    TopDownHUD = Cast<ATopDown_HUD>(GetHUD());
}

void ATopDown_PlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if(!DefaultInputMappingContext)
    {
        return;
    }

    UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());

    if(Subsystem)
    {
        Subsystem->AddMappingContext(DefaultInputMappingContext, 0);
        UE_LOG(LogTemp, Display, TEXT("Input Mapping Context"));
    }

    if(UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
        //Bind Select function to select input action
        EnhancedInputComponent->BindAction(SelectAction, ETriggerEvent::Completed, this, &ATopDown_PlayerController::Select);

        EnhancedInputComponent->BindAction(SelectAction, ETriggerEvent::Started, this, &ATopDown_PlayerController::SelectStart);
        EnhancedInputComponent->BindAction(SelectAction, ETriggerEvent::Triggered, this, &ATopDown_PlayerController::SelectOnGoing);
        EnhancedInputComponent->BindAction(SelectAction, ETriggerEvent::Completed, this, &ATopDown_PlayerController::SelectEnd);

        //Bind Command function 
        EnhancedInputComponent->BindAction(CommandAction, ETriggerEvent::Completed, this, &ATopDown_PlayerController::CommandSelectedActor);
    }
}

void ATopDown_PlayerController::Select(const FInputActionValue& Value)
{
    UE_LOG(LogTemp, Display, TEXT("Select Action"));

    FHitResult HitResult;
    GetHitResultUnderCursor(ECollisionChannel::ECC_Camera, false, HitResult);

    //Deselect previous Pawn
    if(SelectedActor)
    {
        if(SelectedActor->GetClass()->ImplementsInterface(USelectableInterface::StaticClass()))
        {
            ISelectableInterface::Execute_SelectedActor(SelectedActor, false);
        }
    }

    SelectedActor = HitResult.GetActor();

    if(SelectedActor)
    {
        //UE_LOG(LogTemp, Display, TEXT("Selected Actor: %s"), *SelectedActor->GetName());
        
        //select new Pawn
        if(SelectedActor->GetClass()->ImplementsInterface(USelectableInterface::StaticClass()))
        {
            ISelectableInterface::Execute_SelectedActor(SelectedActor, true);
        }

    }
}

void ATopDown_PlayerController::CommandSelectedActor(const FInputActionValue& Value)
{
    FHitResult HitResult;
    GetHitResultUnderCursor(ECollisionChannel::ECC_Camera, false, HitResult);

    if(!HitResult.bBlockingHit)
    {
        
        return;
    }

    if(SelectedActors.Num() > 0)
    {
        int i = SelectedActors.Num() / 2;
        for(AActor* SomeActor : SelectedActors)
        {
            if(SomeActor)
            {
                if(SomeActor->GetClass()->ImplementsInterface(UFactionInterface::StaticClass()))
                {
                    int ActorFaction = IFactionInterface::Execute_GetFaction(SomeActor);
                    if(FactionID != ActorFaction)
                    {
                        continue;
                    }
                }


                if(SomeActor->GetClass()->ImplementsInterface(UNavigableInterface::StaticClass()))
                {
                    INavigableInterface::Execute_MoveToLocation(SomeActor, HitResult.Location + FVector(0, 100 * i, 0));
                    i++;
                }
            }
        }         
    }
    else
    {
        
        if(SelectedActor->GetClass()->ImplementsInterface(UFactionInterface::StaticClass()))
        {   
            int ActorFaction = IFactionInterface::Execute_GetFaction(SelectedActor);
            if(FactionID != ActorFaction)
            {
                return;
            }
        }

        if(SelectedActor->GetClass()->ImplementsInterface(UNavigableInterface::StaticClass()))
        {

            INavigableInterface::Execute_MoveToLocation(SelectedActor, HitResult.Location);        
        }
    }
}

void ATopDown_PlayerController::SelectStart(const FInputActionValue& Value)
{   
    float MouseX, MouseY;
    GetMousePosition(MouseX, MouseY);
    SelectStartPosition = FVector2D(MouseX, MouseY);
    //UE_LOG(LogTemp, Display, TEXT("Selection Start"));
}

void ATopDown_PlayerController::SelectOnGoing(const FInputActionValue& Value)
{
    float MouseX, MouseY;
    GetMousePosition(MouseX, MouseY);
    SelectionSize = FVector2D(MouseX - SelectStartPosition.X, MouseY - SelectStartPosition.Y);

    //UE_LOG(LogTemp, Display, TEXT("Selection OnGoing"));

    if(TopDownHUD)
    {
        TopDownHUD->ShowSelectionRect(SelectStartPosition, SelectionSize);
    }
}

void ATopDown_PlayerController::SelectEnd(const FInputActionValue& Value)
{
    UE_LOG(LogTemp, Display, TEXT("Selection End"));
    if(TopDownHUD)
    {
        TopDownHUD->HideSelectionRect();
        FTimerHandle TimerHandleSelectMultipleActors;
        GetWorld()->GetTimerManager().SetTimer(TimerHandleSelectMultipleActors, this, &ATopDown_PlayerController::SelectMultipleActors, 0.05f, false);
    }

}



void ATopDown_PlayerController::SelectMultipleActors()
{
    
    if(TopDownHUD)
    {
        //Deselect old
        for(AActor* SomeActor : SelectedActors)
        {
            if(SomeActor)
            {
                if(SomeActor)
                {
                    if(SomeActor->GetClass()->ImplementsInterface(USelectableInterface::StaticClass()))
                    {
                        ISelectableInterface::Execute_SelectedActor(SomeActor, false);
                    }
                }
            }
        }         

        SelectedActors.Empty();
        //Select new
        TArray<AActor*> AllSelectedActors = TopDownHUD->GetSelectedActors();
        
        if(AllSelectedActors.Num() == 1)
        {
            AActor* SomeActor = AllSelectedActors[0];
            if(SomeActor->GetClass()->ImplementsInterface(USelectableInterface::StaticClass()))
            {
                ISelectableInterface::Execute_SelectedActor(SomeActor, true);
                SelectedActors.AddUnique(SelectedActor);
            }
            
        }
        else 
        {
            for(AActor* SomeActor : AllSelectedActors)
            {
                if(SomeActor)
                {
                    if(SomeActor->GetClass()->ImplementsInterface(UFactionInterface::StaticClass()))
                    {
                        int32 ActorFaction = IFactionInterface::Execute_GetFaction(SomeActor);
                        if(FactionID != ActorFaction)
                        { 
                            continue;
                        }
                    }                
                    if(SomeActor)
                    {
                        if(SomeActor->GetClass()->ImplementsInterface(USelectableInterface::StaticClass()))
                        {
                            ISelectableInterface::Execute_SelectedActor(SomeActor, true);
                            SelectedActors.AddUnique(SomeActor);
                        }
                    }
                }
            }
        }
        OnActorsSelected.Broadcast(SelectedActors);
    }
}

void ATopDown_PlayerController::SetFaction_Implementation(int32 NewFactionID)
{
    FactionID = NewFactionID;
}

int32 ATopDown_PlayerController::GetFaction_Implementation()
{
    return FactionID;
}

void ATopDown_PlayerController::OnHover_Implementation(bool bIsEnemy)
{
	CurrentMouseCursor = bIsEnemy? EMouseCursor::Crosshairs : EMouseCursor::Hand;
}

void ATopDown_PlayerController::ClearHover_Implementation()
{
    CurrentMouseCursor = EMouseCursor::Default;
}
