// Fill out your copyright notice in the Description page of Project Settings.


#include "ControlPawn.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "TopDown_PlayerController.h"
#include "InputActionValue.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SViewport.h"
#include "Engine/GameViewportClient.h"

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
	// 在缩放相机之前，先获取控制器并中断相机跟随
	if(ATopDown_PlayerController* PlayerController = Cast<ATopDown_PlayerController>(Controller))
	{
		PlayerController->BreakCameraFollow();
	}
	
	const FVector2D MovementInput = Value.Get<FVector2D>();
	if(Controller != nullptr)
	{
		
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		FVector ForwardVector = GetActorForwardVector();
		FVector RightVector = GetActorRightVector();
		
		AddMovementInput(ForwardVector, MovementInput.Y);
		AddMovementInput(RightVector, MovementInput.X);

		//PlayerController->BreakCameraFollow();
	}
}

void AControlPawn::Zoom(const FInputActionValue& Value)
{
	// 在缩放相机之前，先获取控制器并中断相机跟随
	if (ATopDown_PlayerController* PlayerController = Cast<ATopDown_PlayerController>(Controller))
	{
		PlayerController->BreakCameraFollow();
	}
	const float ZoomDirection = Value.Get<float>();
	if(Controller != nullptr)
	{
		float DesiredOrthoWidth = Camera->OrthoWidth - ZoomDirection * CameraZoomSpeed;
		DesiredOrthoWidth = FMath::Clamp(DesiredOrthoWidth, MinOrthWidth, MaxOrthWidth);
		Camera->OrthoWidth = DesiredOrthoWidth;
	}
}

void AControlPawn::RotateCamera(const FInputActionValue& Value)
{	
	// 在旋转相机之前，先获取控制器并中断相机跟随
	if (ATopDown_PlayerController* PlayerController = Cast<ATopDown_PlayerController>(Controller))
	{
		PlayerController->BreakCameraFollow();
	}
	const float RotationValue  = Value.Get<float>();
	if(Controller != nullptr)
	{	
		//FRotator CurrentRotation = SpringArm->GetRelativeRotation();
		//float DeltaYaw  = RotationValue * CameraRotationSpeed * GetWorld()->GetDeltaSeconds();
		//CurrentRotation.Yaw += DeltaYaw;
		//SpringArm->SetRelativeRotation(CurrentRotation);
		AddActorLocalRotation(FRotator(0, RotationValue * CameraRotationSpeed, 0));
	}
}	

void AControlPawn::EdgeWithMouse()
{
	ATopDown_PlayerController* PlayerController = Cast<ATopDown_PlayerController>(Controller);
	if(!PlayerController) return;

    if (GEngine && GEngine->GameViewport)
    {
        // 获取 SViewport 控件的共享指针
        TSharedPtr<SViewport> ViewportWidget = GEngine->GameViewport->GetGameViewportWidget();

        // 检查共享指针是否有效，并且鼠标是否“不”直接悬停在它上面
        if (ViewportWidget.IsValid() && !ViewportWidget->IsHovered())
        {
            // 如果鼠标不是直接悬停在主游戏视口上，
            // 那就意味着它一定在某个UI元素（如小地图）之上。
            // 在这种情况下，我们禁用屏幕边缘滚动。
            return;
        }
    }

	float MouseX = 0, MouseY = 0;
	if(PlayerController->GetMousePosition(MouseX, MouseY))
	{
		
		FVector2D ViewportSize;
		if(GEngine && GEngine->GameViewport)
		{
			GEngine->GameViewport->GetViewportSize(ViewportSize);

			float EdgeThreshold = 200.f;
			FVector2D MovementInput = FVector2D(0, 0);

			if(MouseX < EdgeThreshold && !(MouseY < EdgeThreshold) && !(MouseY > (ViewportSize.Y - EdgeThreshold)))
			{
				UE_LOG(LogTemp, Display, TEXT("Left"));
				MovementInput.X = -1.f * (1.f - MouseX / EdgeThreshold);	//越靠边缘越快 
			}
			if(MouseX > (ViewportSize.X - EdgeThreshold) && !(MouseY < EdgeThreshold) && !(MouseY > (ViewportSize.Y - EdgeThreshold)))
			{
				UE_LOG(LogTemp, Display, TEXT("Rigth"));
				MovementInput.X = 1.f * (MouseX - (ViewportSize.X - EdgeThreshold)) / EdgeThreshold;
			}
			if(MouseY < EdgeThreshold && !(MouseX < EdgeThreshold) && !(MouseX > (ViewportSize.X - EdgeThreshold)))
			{
				UE_LOG(LogTemp, Display, TEXT("Top"));
				MovementInput.Y = 1.f * (1.f - MouseY / EdgeThreshold);
			}
			if(MouseY > (ViewportSize.Y - EdgeThreshold) && !(MouseX < EdgeThreshold) && !(MouseX > (ViewportSize.X - EdgeThreshold)))
			{
				UE_LOG(LogTemp, Display, TEXT("Bottom"));
				MovementInput.Y = -1.f * (MouseY - (ViewportSize.Y - EdgeThreshold)) / EdgeThreshold;
			}

			if(!MovementInput.IsZero())
			{
				//调用了Move函数，所以会自动中断相机跟随，无需额外处理
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
