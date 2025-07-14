// Fill out your copyright notice in the Description page of Project Settings.


#include "CLMBaseBuilding.h"
#include "EnhancedInputComponent.h"
#include "ActorData.h"
#include "InputActionValue.h"
#include "TopDown_PlayerState.h"
#include "Components/BoxComponent.h"


// Sets default values
ACLMBaseBuilding::ACLMBaseBuilding()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	//Create MeshComponent
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	SetRootComponent(StaticMesh);

	//Create Selected Indicator
	SelectedIndicator = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SelectedIndicator"));
	SelectedIndicator->SetupAttachment(StaticMesh);
	SelectedIndicator->SetHiddenInGame(true);
	SelectedIndicator->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Called when the game starts or when spawned
void ACLMBaseBuilding::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACLMBaseBuilding::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	
	
}

void ACLMBaseBuilding::SelectedActor_Implementation(const bool Selected)
{
	SelectedIndicator->SetHiddenInGame(!Selected);
}

void ACLMBaseBuilding::SetFaction_Implementation(int32 NewFactionID)
{
	FactionID = NewFactionID;
}

int32 ACLMBaseBuilding::GetFaction_Implementation()
{
	return FactionID;
}

void ACLMBaseBuilding::EnablePlaceMode()
{
	UE_LOG(LogTemp, Display, TEXT("Enable Place Mode"));
	//Enable Inputs on	 this actor
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();

	if(PlayerController)
	{
		//Add Input Mapping Context
		UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerController->InputComponent);
		if(EnhancedInputComponent)
		{
			//Add Input Mapping Context
			EnhancedInputComponent->BindAction(PlaceAction, ETriggerEvent::Completed, this, &ACLMBaseBuilding::PlaceBuilding);

		}
	}

	GetWorld()->GetTimerManager().SetTimer(PlaceBuildingTimerHandle, this, &ACLMBaseBuilding::CheckPlaceValidity, 0.03f, true);
	this->SetActorEnableCollision(false);
}

void ACLMBaseBuilding::CheckPlaceValidity()
{
	UE_LOG(LogTemp, Display, TEXT("Check Place Validity"));

	ToggleBuildingValidity(bCanPlacing);

	bCanPlacing = false;

	//Set building Location under cursor(follow)
	FHitResult HitResult;
	GetWorld()->GetFirstPlayerController()->GetHitResultUnderCursor(ECC_Visibility, false, HitResult);
	if(!HitResult.bBlockingHit)
	{
		UE_LOG(LogTemp, Warning, TEXT("No Hit Result"));
		return;
	}
	SetActorLocation(HitResult.Location);

	if(!HitResult.GetActor()->ActorHasTag(RequiredTag))
	{
		UE_LOG(LogTemp, Warning, TEXT("No Valid Place"));
		return;
	}

	//Define parameters for box sweep
	FVector BoxHalfExtent = BuildingExtends / 2;
	FVector TraceStart = HitResult.Location + FVector(0.f, 0.f, BoxHalfExtent.Z);
	FVector TraceEnd = TraceStart + FVector::UpVector;

	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);
	CollisionParams.bTraceComplex = false;
	TArray<FHitResult> OutHits;

	//Perform a box sweep to check for collisions
	bool bHit = GetWorld()->SweepMultiByChannel(OutHits, TraceStart, TraceEnd, GetActorRotation().Quaternion(), ECC_Visibility, FCollisionShape::MakeBox(BoxHalfExtent), CollisionParams);

	for(const FHitResult& Hit : OutHits)
	{
		if(!(Hit.GetActor() != nullptr && Hit.GetActor()->ActorHasTag(RequiredTag)))
		{
			//UE_LOG(LogTemp, Warning, TEXT("Area blocked by: %s"), *Hit.GetActor()->GetName());
			return;
		}
	}

	bCanPlacing = true;
	
}

void ACLMBaseBuilding::PlaceBuilding(const FInputActionValue& Value)
{
	if(!bCanPlacing)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot Place"));
		RevertBuildingCast();
		SetLifeSpan(0.1f);	//Destory if cannot place
		return;
	}
	
	UE_LOG(LogTemp, Display, TEXT("Place Building"));

	//clear timer handle
	GetWorld()->GetTimerManager().ClearTimer(PlaceBuildingTimerHandle);
	this->SetActorEnableCollision(true);
}

void ACLMBaseBuilding::CancelBuilding(const FInputActionValue& Value)	
{
	UE_LOG(LogTemp, Display, TEXT("Cancel Building"));
}

void ACLMBaseBuilding::RevertBuildingCast()
{
	if(!DT_BuildCost)
	{
		UE_LOG(LogTemp, Warning, TEXT("No Build Cost Data Table"));
		return;
	}
	
	FString ActorTypeStr = UEnum::GetValueAsString(ActorType);
	FString CleanNameStr;
	ActorTypeStr.Split(TEXT("::"), nullptr, &CleanNameStr);
	FBuildCastData* BuildCast_S = DT_BuildCost->FindRow<FBuildCastData>(FName(CleanNameStr), TEXT(""));

	if(BuildCast_S)
	{
		ATopDown_PlayerState* PlayerState = Cast<ATopDown_PlayerState>(GetWorld()->GetFirstPlayerController()->PlayerState);
		PlayerState->RevertBuildCast(BuildCast_S->BuildCast);	
	}
	
}

EActorType ACLMBaseBuilding::GetActorType_Implementation()
{
    return ActorType;
}

TArray<TSubclassOf<ACLMBaseBuilding>> ACLMBaseBuilding::GetBuildingOptions_Implementation()
{
    return BuildingOptions;
}