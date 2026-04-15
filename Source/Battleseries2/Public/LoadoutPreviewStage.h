// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Vehicle_Base.h"
#include "LoadoutPreviewStage.generated.h"

struct FAttachmentState;

USTRUCT(BlueprintType)
struct FPreviewState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)		//the mode/generic category of thing we are customizing
	ECoreType CurrentStageMode = ECoreType::Vehicle;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	AVehicle_Base* CurrentVehicle = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)				//attachment state of preview gun
	TArray<FAttachmentState> PreviewGun_AttachmentState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool RotateObject = false;		//true = rotate object, false = orbit camera

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	ACameraActor* LoadoutCam = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	ACameraActor* ItemCam = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ACameraActor* VehicleCam = nullptr;
};

UCLASS()
class BATTLESERIES2_API ALoadoutPreviewStage : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALoadoutPreviewStage();

	//Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* Root = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* VehicleAnchor = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* GarageMesh = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* VehicleBoom = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* LoadoutBoom = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* ItemBoom = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	USkeletalMeshComponent* PreviewGun = nullptr;


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	struct FPreviewState PreviewStageState = FPreviewState();

	//if we're gonna expand to not just be spawning vehicle but also regular weapons, need additional functions (consider interface)
	UFUNCTION(BlueprintCallable)
	void UpdateWeaponPreview(FName WeaponID);
	UFUNCTION(BlueprintCallable)
	void SetupNewPreviewVehicle(FTransform PreviewTransformOffset, FVehicleStartingData InputVehicleStartingData);
	UFUNCTION(BlueprintCallable)
	void CenterCameraOnVehicle();
	UFUNCTION(BlueprintCallable)
	void UpdateCameraBoomLength(USpringArmComponent* CamBoom, float DeltaZoom, float ZoomSpeed, float MinZoom, float MaxZoom);

	UFUNCTION(BlueprintCallable)
	void RotatePreview(float DeltaYaw, float DeltaPitch);
	UFUNCTION(BlueprintCallable)
	void ZoomPreview(float DeltaZoom);

	UFUNCTION(BlueprintCallable)
	FName GetVehicleMeshRootBoneName();
	UFUNCTION(BlueprintCallable, BlueprintPure)
	AActor* GetCurrentPreviewCameraActor();
	UFUNCTION(BlueprintCallable, BlueprintPure)
	UDA_CustomizationDefaults* GetCustomizationSystemDefaults();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;



public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
