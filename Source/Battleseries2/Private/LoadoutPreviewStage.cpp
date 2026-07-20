// Fill out your copyright notice in the Description page of Project Settings.


#include "LoadoutPreviewStage.h"
#include "Vehicle_Base.h"
#include "Data/Data_Customization.h"
#include "Data/Weapons/Data_InfantryWeapon.h"
#include "Utilities/BS2FunctionLibrary.h"
#include "Utilities/DataManagerSubsystem.h"
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
	PreviewGun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PreviewGun"));
	ItemBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("ItemBoom"));
	VehicleBoom->SetupAttachment(VehicleAnchor, FName(""));
	LoadoutBoom->SetupAttachment(Root, FName(""));
	PreviewGun->SetupAttachment(Root);
	ItemBoom->SetupAttachment(PreviewGun, FName(""));
}

// Called when the game starts or when spawned
void ALoadoutPreviewStage::BeginPlay()
{
	Super::BeginPlay();

	//assumes vehicle ca comp is valid and is a vehicle
	FActorSpawnParameters VehicleSpawnParams;
	VehicleSpawnParams.Owner = this;
	VehicleSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	PreviewStageState.CurrentVehicle = GetWorld()->SpawnActor<AVehicle_Base>(UBS2FunctionLibrary::GetDataSubsystem(GetWorld())->GetCustomizationDefaults()->PreviewVehicleClass, FTransform::Identity, VehicleSpawnParams);
	PreviewStageState.CurrentVehicle->VehicleStartingData.PreviewVehicle = true;
	if (USkeletalMeshComponent* VehicleMesh = PreviewStageState.CurrentVehicle->VehicleMeshComponent)
	{
		VehicleMesh->SetSimulatePhysics(false);
		VehicleMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Prevent landscape snapping
	}

	// 2. CRITICAL: Disable the vehicle movement component updates entirely
	if (UChaosVehicleMovementComponent* VehicleMovement = PreviewStageState.CurrentVehicle->ChaosVehicleMovement)
	{
		// Deactivating stops Chaos/PhysX from trying to ground-snap the chassis
		VehicleMovement->Deactivate();
	}
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

void ALoadoutPreviewStage::UpdateWeaponPreview(FName WeaponID)
{
	//probably move this to some weapon logic comp later
	const FInfantryWeaponData* WeaponData = UBS2FunctionLibrary::GetDataSubsystem(GetWorld())->GetInfantryWeaponDataRow(WeaponID);
	WeaponData->WeaponClassificationData.WeaponMesh.LoadSynchronous();
	if (WeaponData->WeaponClassificationData.WeaponMesh)
	{
		PreviewGun->SetSkeletalMesh(WeaponData->WeaponClassificationData.WeaponMesh.Get());
	}
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

void ALoadoutPreviewStage::UpdateCameraBoomLength(USpringArmComponent* CamBoom, float DeltaZoom, float ZoomSpeed, float MinZoom, float MaxZoom)
{
	float NewLength;
	NewLength = FMath::Clamp(CamBoom->TargetArmLength - DeltaZoom * ZoomSpeed, MinZoom, MaxZoom);
	CamBoom->TargetArmLength = NewLength;
}

void ALoadoutPreviewStage::RotatePreview(float DeltaYaw, float DeltaPitch)
{
	FRotator NewRotation;
	float RotateSpeed = UBS2FunctionLibrary::GetDataSubsystem(GetWorld())->GetCustomizationDefaults()->CustomizationModeCamSettings.Find(PreviewStageState.CurrentStageMode)->RotateSpeed;
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
	float ZoomSpeed = UBS2FunctionLibrary::GetDataSubsystem(GetWorld())->GetCustomizationDefaults()->CustomizationModeCamSettings.Find(PreviewStageState.CurrentStageMode)->ZoomSpeed;
	float MinZoom = UBS2FunctionLibrary::GetDataSubsystem(GetWorld())->GetCustomizationDefaults()->CustomizationModeCamSettings.Find(PreviewStageState.CurrentStageMode)->MinZoom;
	float MaxZoom = UBS2FunctionLibrary::GetDataSubsystem(GetWorld())->GetCustomizationDefaults()->CustomizationModeCamSettings.Find(PreviewStageState.CurrentStageMode)->MaxZoom;

	switch (PreviewStageState.CurrentStageMode)
	{
		case ECoreType::Vehicle:
			UpdateCameraBoomLength(VehicleBoom, DeltaZoom, ZoomSpeed, MinZoom, MaxZoom);
			break;
		case ECoreType::Class:
			UpdateCameraBoomLength(LoadoutBoom, DeltaZoom, ZoomSpeed, MinZoom, MaxZoom);
			break;
		case ECoreType::Weapon:
			UpdateCameraBoomLength(ItemBoom, DeltaZoom, ZoomSpeed, MinZoom, MaxZoom);
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