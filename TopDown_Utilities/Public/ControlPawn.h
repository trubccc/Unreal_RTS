// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ControlPawn.generated.h"

class UCapsuleComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;
class UFloatingPawnMovement;




UCLASS()
class TOPDOWN_UTILITIES_API AControlPawn : public APawn
{
	GENERATED_BODY()

private:
	//Capsual Component
	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = Collision, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCapsuleComponent> CapsuleComponent;

	//Camera Boom
	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> SpringArm;
	
	//Camera
	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> Camera;

	//Camera Zoom
	UPROPERTY(EditAnyWhere, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float CameraZoomSpeed = 10.f;
	UPROPERTY(EditAnyWhere, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float MaxOrthWidth = 3500.f;
	UPROPERTY(EditAnyWhere, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float MinOrthWidth = 300.f;
	//Camera Rotation
	UPROPERTY(EditAnyWhere, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float CameraRotationSpeed = 10.f;


	//Move Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveAction;

	//Zoom Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ZoomAction;

	//Rotate Camera 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> RotateCameraAction;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UFloatingPawnMovement> FloatingPawnAction;

public:
	// Sets default values for this pawn's properties
	AControlPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	//movement input
	void Move(const FInputActionValue& Value);

	//zoom input
	void Zoom(const FInputActionValue& Value);

	//rotate camera input
	void RotateCamera(const FInputActionValue& Value);

	//Edge with mouse
	void EdgeWithMouse();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	// Called to bind functionality to input
	UFUNCTION()
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	

};
