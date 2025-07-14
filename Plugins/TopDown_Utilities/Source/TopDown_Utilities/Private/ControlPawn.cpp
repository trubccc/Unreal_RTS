// Fill out your copyright notice in the Description page of Project Settings.


#include "ControlPawn.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "InputActionValue.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/FloatingPawnMovement.h"


// Sets default values
AControlPawn::AControlPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Create Capsule
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	SetRootComponent(CapsuleComponent);

	//Create SpringArm
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);

	//Create Camera
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	
	//Create Floating Movement Component
	FloatingPawnAction = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("FloatingPawnMovement"));
	

}

// Called when the game starts or when spawned
void AControlPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

void AControlPawn::Move(const FInputActionValue& Value)
{
	const FVector2D MovementInput = Value.Get<FVector2D>();
	if(Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(Forward, MovementInput.Y);
		AddMovementInput(Right, MovementInput.X);
	}
}

void AControlPawn::Zoom(const FInputActionValue& Value)
{
	const float ZoomDirection = Value.Get<float>();
	if(Controller != nullptr)
	{
		float DesiredOrthoWidth = Camera->OrthoWidth + ZoomDirection * CameraZoomSpeed;
		DesiredOrthoWidth = FMath::Clamp(DesiredOrthoWidth, MinOrthWidth, MaxOrthWidth);
		Camera->OrthoWidth = DesiredOrthoWidth;
	}
}

void AControlPawn::RotateCamera(const FInputActionValue &Value)
{	
	const float RotationValue  = Value.Get<float>();
	if(Controller != nullptr)
	{	
		FRotator CurrentRotation = SpringArm->GetRelativeRotation();
		float DeltaYaw  = RotationValue * CameraRotationSpeed * GetWorld()->GetDeltaSeconds();
		CurrentRotation.Yaw += DeltaYaw;
		SpringArm->SetRelativeRotation(CurrentRotation);
	}
}	

void AControlPawn::EdgeWithMouse()
{
	APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if(!PlayerController) return;

	float MouseX = 0, MouseY = 0;
	if(PlayerController->GetMousePosition(MouseX, MouseY))
	{
		FVector2D ViewportSize;
		if(GEngine && GEngine->GameViewport)
		{
			GEngine->GameViewport->GetViewportSize(ViewportSize);

			float EdgeThreshold = 10.f;
			FVector2D MovementInput = FVector2D(0, 0);

			if(MouseX < EdgeThreshold)
			{
				UE_LOG(LogTemp, Display, TEXT("Left"));
				MovementInput.X = -1.f;
			}
			if(MouseX > (ViewportSize.X - EdgeThreshold))
			{
				UE_LOG(LogTemp, Display, TEXT("Rigth"));
				MovementInput.X = 1.f;
			}
			if(MouseY < EdgeThreshold)
			{
				UE_LOG(LogTemp, Display, TEXT("Top"));
				MovementInput.Y = 1.f;
			}
			if(MouseY > (ViewportSize.Y - EdgeThreshold))
			{
				UE_LOG(LogTemp, Display, TEXT("Bottom"));
				MovementInput.Y = -1.f;
			}

			if(!MovementInput.IsZero())
			{
				Move(FInputActionValue(MovementInput));
			}
		}
	}
}

// Called every frame
void AControlPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	EdgeWithMouse();

}

// Called to bind functionality to input
void AControlPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if(UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Bind Move function to move input action
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AControlPawn::Move);

		//Bind Zoom function to zoom input action
		EnhancedInputComponent->BindAction(ZoomAction, ETriggerEvent::Triggered, this, &AControlPawn::Zoom);

		//Bind Rotate Camera function to rotate camera input action
		EnhancedInputComponent->BindAction(RotateCameraAction, ETriggerEvent::Triggered, this, &AControlPawn::RotateCamera);
	}

}
