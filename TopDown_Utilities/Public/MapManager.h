// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MapManager.generated.h"

class USceneCaptureComponent2D;
class USpringArmComponent;

UCLASS()
class TOPDOWN_UTILITIES_API AMapManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMapManager();
private:
	//Map Dimensions
	UPROPERTY(EditAnywhere, Category = "Map Settings")
	FVector2D MapDimensions = FVector2D(100000.f, 100000.f);

	//Map capture height
	UPROPERTY(EditAnywhere, Category = "Map Settings")
	float MapCaptureHeight = 1000.f;

	//Capture
	UPROPERTY(EditAnywhere, Category = "Map Settings")
	USceneCaptureComponent2D* SceneCaptureComponent2D;

	//Texture to the map
	UPROPERTY(EditAnywhere, Category = "Map Settings")
	UTextureRenderTarget2D* RTMapTexture;

	//Spring arm 
	UPROPERTY(EditAnywhere, Category = "Map Settings")
	USpringArmComponent* SpringArmComponent;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//Initialize Map
	void InitializeSceneCaptureComponent2D();

	//capture the map
	UFUNCTION(BlueprintCallable, Category = "Map", CallInEditor)
	void CaptureMapTexture();

};
