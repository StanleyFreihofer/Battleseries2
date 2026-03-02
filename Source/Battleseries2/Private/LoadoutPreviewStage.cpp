// Fill out your copyright notice in the Description page of Project Settings.


#include "LoadoutPreviewStage.h"
#include "Vehicle_Base.h"


// Sets default values
ALoadoutPreviewStage::ALoadoutPreviewStage()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	VehicleAnchor = CreateDefaultSubobject<USceneComponent>(TEXT("VehicleAnchor"));
	RootComponent = VehicleAnchor;
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));

	SpringArm->SetupAttachment(VehicleAnchor, FName(""));
	Camera->SetupAttachment(SpringArm, FName("SpringEndpoint"));
}

// Called when the game starts or when spawned
void ALoadoutPreviewStage::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ALoadoutPreviewStage::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//FVector Start = SpringArm->GetComponentLocation();
	//FVector End = Camera->GetComponentLocation();
	//DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, -1.f, 0, 2.f);
}

void ALoadoutPreviewStage::SetupNewPreviewVehicle(FTransform PreviewTransformOffset, FVehicleStartingData InputVehicleStartingData)
{
	FTransform AnchorTransform = VehicleAnchor->GetComponentTransform();
	FTransform OffsetTransform = PreviewTransformOffset;
	FTransform FinalTransform = OffsetTransform * AnchorTransform;
	UE_LOG(LogTemp, Warning, TEXT("location: %s"), *FinalTransform.GetLocation().ToString());

	if (CurrentVehicle)
	{
		CurrentVehicle->SetActorTransform(FinalTransform);
		CurrentVehicle->SetVehicleAndInit(InputVehicleStartingData);

	}
	else
	{
		CurrentVehicle = GetWorld()->SpawnActor<AVehicle_Base>(VehicleClass, FinalTransform);
		if (CurrentVehicle)
		{
			CurrentVehicle->SetVehicleAndInit(InputVehicleStartingData);
		}
		else
		{
			FString ClassName = VehicleClass ? VehicleClass->GetName() : TEXT("NULL CLASS");
			UE_LOG(LogTemp, Error, TEXT("[SaveSubsystem] Failed to spawn VehicleClass: %s"), *VehicleClass->GetName());
		}
		//CurrentVehicle->SetVehicleAndInit(InputVehicleStartingData);
	}
	CurrentObject = CurrentVehicle;
	CurrentVehicle->OnMeshReady.RemoveDynamic(this, &ALoadoutPreviewStage::CenterCameraOnVehicle);
	CurrentVehicle->OnMeshReady.AddDynamic(this, &ALoadoutPreviewStage::CenterCameraOnVehicle);
}

void ALoadoutPreviewStage::CenterCameraOnVehicle()
{
	FName RootBoneToAttachTo = GetVehicleMeshRootBoneName();
	SpringArm->AttachToComponent(CurrentVehicle->VehicleMeshComponent, FAttachmentTransformRules::SnapToTargetIncludingScale, RootBoneToAttachTo);
	Camera->AttachToComponent(SpringArm, FAttachmentTransformRules::SnapToTargetIncludingScale);
	//SpringArm->SetRelativeRotation(FRotator::ZeroRotator);
	SpringArm->SetRelativeLocation(FVector(0, 0, 0)); // height above vehicle
	SpringArm->SetRelativeRotation(FRotator(-30, -150,0)); // slight tilt
}

void ALoadoutPreviewStage::RotatePreview(float DeltaYaw, float DeltaPitch)
{
	if (RotateObject)
	{
		// Rotate the mesh locally
		FRotator NewRotation = CurrentObject->GetActorRotation();
		NewRotation.Yaw += DeltaYaw * RotateSpeed;
		NewRotation.Pitch = FMath::Clamp(NewRotation.Pitch + DeltaPitch * RotateSpeed, -90.f, 90.f);
		CurrentObject->SetActorRelativeRotation(NewRotation);
	}
	else
	{
		// Rotate the camera (spring arm) around mesh
		FRotator NewRotation = SpringArm->GetRelativeRotation();
		NewRotation.Yaw += DeltaYaw * RotateSpeed;
		NewRotation.Pitch = FMath::Clamp(NewRotation.Pitch + DeltaPitch * RotateSpeed, -90.f, 45.f);
		NewRotation.Roll = FMath::Clamp(NewRotation.Roll, 0.f, 0.f); // clamp roll
		SpringArm->SetRelativeRotation(NewRotation);
	}
}

void ALoadoutPreviewStage::ZoomPreview(float DeltaZoom)
{
	float NewLength = FMath::Clamp(SpringArm->TargetArmLength - DeltaZoom * ZoomSpeed, MinZoom, MaxZoom);
	SpringArm->TargetArmLength = NewLength;
}

FName ALoadoutPreviewStage::GetVehicleMeshRootBoneName()
{
	if (!CurrentVehicle->VehicleMeshComponent->GetSkeletalMeshAsset())
	{
		UE_LOG(LogTemp, Warning, TEXT("No vehicle to center camera on!"));
		return NAME_None;
	}
	USkeletalMesh* VehicleMesh = CurrentVehicle->VehicleMeshComponent->GetSkeletalMeshAsset();
	FName RootBoneName = VehicleMesh->GetRefSkeleton().GetBoneName(0);
	UE_LOG(LogTemp, Warning, TEXT("RootBoneName = %s"), *RootBoneName.ToString());
	return RootBoneName;
}


