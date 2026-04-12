// Fill out your copyright notice in the Description page of Project Settings.


#include "LoadoutPreviewStage.h"
#include "Vehicle_Base.h"
#include "Camera/CameraActor.h"


// Sets default values
ALoadoutPreviewStage::ALoadoutPreviewStage()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	GarageMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GarageStage"));
	VehicleAnchor = CreateDefaultSubobject<USceneComponent>(TEXT("VehicleAnchor"));

	SetRootComponent(Root);
	GarageMesh->SetupAttachment(Root);
	VehicleAnchor->SetupAttachment(Root);
	VehicleBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("VehicleBoom"));
	LoadoutBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("LoadoutBoom"));
	ItemBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("ItemBoom"));
	VehicleBoom->SetupAttachment(VehicleAnchor, FName(""));
	LoadoutBoom->SetupAttachment(Root, FName(""));
	ItemBoom->SetupAttachment(Root, FName(""));
}

// Called when the game starts or when spawned
void ALoadoutPreviewStage::BeginPlay()
{
	Super::BeginPlay();

	//assumes vehicle ca comp is valid and is a vehicle
	PreviewStageState.CurrentVehicle = GetWorld()->SpawnActorDeferred<AVehicle_Base>(GetGameInstance()->GetSubsystem<UDataManagerSubsystem>()->GetCustomizationDefaults()., FTransform::Identity);
	PreviewStageState.CurrentVehicle->VehicleStartingData.PreviewVehicle = true;
	PreviewStageState.CurrentVehicle->AttachToComponent(VehicleAnchor, FAttachmentTransformRules::KeepRelativeTransform);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();

	PreviewStageState.VehicleCam = GetWorld()->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), FTransform::Identity, SpawnParams);
	PreviewStageState.VehicleCam->AttachToComponent(VehicleBoom, FAttachmentTransformRules::KeepRelativeTransform, FName("SpringEndpoint"));
	PreviewStageState.VehicleCam->GetCameraComponent()->bConstrainAspectRatio = false;
	PreviewStageState.LoadoutCam = GetWorld()->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), FTransform::Identity, SpawnParams);
	PreviewStageState.LoadoutCam->AttachToComponent(LoadoutBoom, FAttachmentTransformRules::KeepRelativeTransform, FName("SpringEndpoint"));
	PreviewStageState.LoadoutCam->GetCameraComponent()->bConstrainAspectRatio = false;
	PreviewStageState.ItemCam = GetWorld()->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), FTransform::Identity, SpawnParams);
	PreviewStageState.ItemCam->AttachToComponent(ItemBoom, FAttachmentTransformRules::KeepRelativeTransform, FName("SpringEndpoint"));
	PreviewStageState.ItemCam->GetCameraComponent()->bConstrainAspectRatio = false;
	CenterCameraOnVehicle();
}

// Called every frame
void ALoadoutPreviewStage::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ALoadoutPreviewStage::SetupNewPreviewVehicle(FTransform PreviewTransformOffset, FVehicleStartingData InputVehicleStartingData)
{
	PreviewStageState.CurrentVehicle->SetVehicleAndInit(InputVehicleStartingData);
	PreviewStageState.CurrentVehicle->SetActorRelativeTransform(PreviewTransformOffset);
}

void ALoadoutPreviewStage::CenterCameraOnVehicle()
{
	VehicleBoom->SetRelativeLocation(FVector(0, 0, 0)); // height above vehicle
	VehicleBoom->SetRelativeRotation(FRotator(-30, -150,0)); // slight tilt
}

void ALoadoutPreviewStage::RotatePreview(float DeltaYaw, float DeltaPitch)
{
	FRotator NewRotation;
	if (PreviewStageState.RotateObject)
	{
		// Rotate the mesh locally
		switch (PreviewStageState.CurrentStageMode)
		{
			case ECoreType::Vehicle:
				NewRotation = PreviewStageState.CurrentVehicle->GetActorRotation();
				NewRotation.Yaw += DeltaYaw * RotateSpeed;
				NewRotation.Pitch = FMath::Clamp(NewRotation.Pitch + DeltaPitch * RotateSpeed, -90.f, 90.f);
				PreviewStageState.CurrentVehicle->SetActorRelativeRotation(NewRotation);
				break;
			case ECoreType::Class:
				break;
		}

	}
	else
	{
		// Rotate the camera (spring arm) around mesh
		switch (PreviewStageState.CurrentStageMode)
		{
			case ECoreType::Vehicle:
				NewRotation = VehicleBoom->GetRelativeRotation();
				NewRotation.Yaw += DeltaYaw * RotateSpeed;
				NewRotation.Pitch = FMath::Clamp(NewRotation.Pitch + DeltaPitch * RotateSpeed, -90.f, 45.f);
				NewRotation.Roll = FMath::Clamp(NewRotation.Roll, 0.f, 0.f); // clamp roll
				VehicleBoom->SetRelativeRotation(NewRotation);
				break;
			case ECoreType::Class:
				NewRotation = LoadoutBoom->GetRelativeRotation();
				NewRotation.Yaw += DeltaYaw * RotateSpeed;
				NewRotation.Pitch = FMath::Clamp(NewRotation.Pitch + DeltaPitch * RotateSpeed, -90.f, 45.f);
				NewRotation.Roll = FMath::Clamp(NewRotation.Roll, 0.f, 0.f); // clamp roll
				LoadoutBoom->SetRelativeRotation(NewRotation);
				break;
		}
	}
}

void ALoadoutPreviewStage::ZoomPreview(float DeltaZoom)
{
	switch (PreviewStageState.CurrentStageMode)
	{
		float NewLength;
		case ECoreType::Vehicle:
			NewLength = FMath::Clamp(VehicleBoom->TargetArmLength - DeltaZoom * ZoomSpeed, MinZoom, MaxZoom);
			VehicleBoom->TargetArmLength = NewLength;
			break;
		case ECoreType::Class:
			NewLength = FMath::Clamp(LoadoutBoom->TargetArmLength - DeltaZoom * ZoomSpeed, MinZoom, MaxZoom);
			LoadoutBoom->TargetArmLength = NewLength;
		case ECoreType::Weapon:
			NewLength = FMath::Clamp(LoadoutBoom->TargetArmLength - DeltaZoom * ZoomSpeed, MinZoom, MaxZoom);
			break;
	}
}

FName ALoadoutPreviewStage::GetVehicleMeshRootBoneName()
{
	if (!PreviewStageState.CurrentVehicle->VehicleMeshComponent->GetSkeletalMeshAsset())
	{
		UE_LOG(LogTemp, Warning, TEXT("No vehicle to center camera on!"));
		return NAME_None;
	}
	USkeletalMesh* VehicleMesh = PreviewStageState.CurrentVehicle->VehicleMeshComponent->GetSkeletalMeshAsset();
	FName RootBoneName = VehicleMesh->GetRefSkeleton().GetBoneName(0);
	UE_LOG(LogTemp, Warning, TEXT("RootBoneName = %s"), *RootBoneName.ToString());
	return RootBoneName;
}

AActor* ALoadoutPreviewStage::GetCurrentPreviewCameraActor()
{
	//use this to get a view target
	switch (PreviewStageState.CurrentStageMode)
	{
		case ECoreType::Character:
			return nullptr;
			break;
		case ECoreType::Class:
			return PreviewStageState.LoadoutCam;
			break;
		case ECoreType::Weapon:
			return PreviewStageState.ItemCam;
			break;
		case ECoreType::Vehicle:
			return PreviewStageState.VehicleCam;
			break;
	}
	return nullptr;
}


