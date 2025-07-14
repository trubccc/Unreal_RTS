// Fill out your copyright notice in the Description page of Project Settings.


#include "CLMBasePawn.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Math/UnrealMathUtility.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameplayMessageStructures.h"
#include "AIController.h"



// Sets default values
ACLMBasePawn::ACLMBasePawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Create Capsule 
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	SetRootComponent(CapsuleComponent);
	CapsuleComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);

	//Create Skeletal Mesh
	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(CapsuleComponent);

	//Create Movement
	FloatPawnMovement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("FloatPawnMovement"));
	

	//Create selected indicator
	SelectedIndicator = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SelectedIndicator"));
	SelectedIndicator->SetupAttachment(CapsuleComponent);
	SelectedIndicator->SetHiddenInGame(true);
	SelectedIndicator->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Called when the game starts or when spawned
void ACLMBasePawn::BeginPlay()
{
	Super::BeginPlay();
	
	//Bind Mouse Event
	CapsuleComponent->OnBeginCursorOver.AddDynamic(this, &ACLMBasePawn::OnCapsuleBeginCursorOver);
	CapsuleComponent->OnEndCursorOver.AddDynamic(this, &ACLMBasePawn::OnCapsuleEndCursorOver);

}

void ACLMBasePawn::Move()
{
	if(!bMoving)
	{
		return;
	}

	//Get direction to move
	FVector MoveDirection = (MoveTargerLocation - GetActorLocation());

	//Move in that direction
	if(MoveDirection.Length() < AcceptanceRadius)
	{
		bMoving = false;
		return;
	}

	MoveDirection.Normalize(1);
	//AddMovementInput(MoveDirection, 1.f);

	FRotator DirectRotation = UKismetMathLibrary::MakeRotFromX(MoveDirection);
	DirectRotation.Pitch = 0;
	DirectRotation.Roll = 0;

	FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), DirectRotation, GetWorld()->DeltaTimeSeconds, TurnSpeed);
	SetActorRotation(NewRotation);

}

// Called every frame
void ACLMBasePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Move();
}

// Called to bind functionality to input
void ACLMBasePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ACLMBasePawn::SelectedActorLocal(const bool IsSelected)
{
	SelectedIndicator->SetHiddenInGame(!IsSelected);
}

void ACLMBasePawn::SelectedActor_Implementation(const bool Selected)
{
	SelectedIndicator->SetHiddenInGame(!Selected);
}

void ACLMBasePawn::MoveToLocation_Implementation(const FVector TargetLocation)
{
	
	UE_LOG(LogTemp, Display, TEXT("Navigating"));

	MoveTargerLocation = TargetLocation + FVector(0, 0, GetDefaultHalfHeight());
	bMoving = true;
	AAIController* PawnAIController = Cast<AAIController>(GetController());
	PawnAIController->MoveToLocation(MoveTargerLocation, AcceptanceRadius);
}

EActorType ACLMBasePawn::GetActorType_Implementation()
{
    return PawnType;
}

TArray<TSubclassOf<ACLMBaseBuilding>> ACLMBasePawn::GetBuildingOptions_Implementation()
{
    return BuildingOptions;
}

void ACLMBasePawn::SetFaction_Implementation(int32 Newfaction)
{
	FactionID = Newfaction;
}

int32 ACLMBasePawn::GetFaction_Implementation()
{
    return FactionID;
}

void ACLMBasePawn::OnHover_Implementation(bool bIsEnemy)
{
	IsEnemy = bIsEnemy;
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	PlayerController->CurrentMouseCursor = bIsEnemy? EMouseCursor::Crosshairs : EMouseCursor::Hand;
}

void ACLMBasePawn::ClearHover_Implementation()
{
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	PlayerController->CurrentMouseCursor = EMouseCursor::Default;	
}

void ACLMBasePawn::OnCapsuleBeginCursorOver(UPrimitiveComponent *TouchedComponent)
{
	UE_LOG(LogTemp, Display, TEXT("OnCapsuleBeginCursorOver"));

	//Gameplay messsage tag
	FGameplayTag MessageTag = FGameplayTag::RequestGameplayTag(FName("Actor.Events.BeginCursorOver"));

	//Create message
	FCommonGamePlayMessage Message;
	Message.Sender = this;

	OnHover_Implementation(IsEnemy);

	//Broadcast the message
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	MessageSubsystem.BroadcastMessage(MessageTag, Message);
}

void ACLMBasePawn::OnCapsuleEndCursorOver(UPrimitiveComponent *TouchedComponent)
{
	UE_LOG(LogTemp, Display, TEXT("OnCapsuleEndCursorOver"));

	//Gameplay messsage tag
	FGameplayTag MessageTag = FGameplayTag::RequestGameplayTag(FName("Actor.Events.EndCursorOver"));

	//Create message
	FCommonGamePlayMessage Message;
	Message.Sender = this;

	ClearHover_Implementation();
	//Broadcast the message
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	MessageSubsystem.BroadcastMessage(MessageTag, Message);
}	

