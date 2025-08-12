// Fill out your copyright notice in the Description page of Project Settings.


#include "MapManager.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/SceneCaptureComponent2D.h"


// Sets default values
AMapManager::AMapManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Create root
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	SetRootComponent(RootComponent);

	//Create Spring arm
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(RootComponent);

	SceneCaptureComponent2D = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCaptureComponent2D"));
	SceneCaptureComponent2D->SetupAttachment(SpringArmComponent);

	//Initialize
	InitializeSceneCaptureComponent2D();

	//capture
	CaptureMapTexture();
}

// Called when the game starts or when spawned
void AMapManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMapManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMapManager::InitializeSceneCaptureComponent2D()
{
	UE_LOG(LogTemp, Warning, TEXT("Initializing SceneCaptureComponent2D"));
	//SceneCaptureComponent2D->SetRelativeLocation(FVector(0.f, 0.f, MapCaptureHeight));
	//SceneCaptureComponent2D->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));

	SceneCaptureComponent2D->ProjectionType = ECameraProjectionMode::Orthographic;
	SceneCaptureComponent2D->OrthoWidth = MapDimensions.X;
	SceneCaptureComponent2D->bCaptureEveryFrame = false;
	SceneCaptureComponent2D->TextureTarget = RTMapTexture;

}

void AMapManager::CaptureMapTexture()
{
	UE_LOG(LogTemp, Warning, TEXT("Capturing Map"));
	InitializeSceneCaptureComponent2D();
	SceneCaptureComponent2D->CaptureScene();
}
