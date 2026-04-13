#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Vehicle_Base.h"
#include "Data_Customization.generated.h"

enum class ECoreType : uint8;
enum class EVehicleType : uint8;
class UUW_Customization;
class ALoadoutPreviewStage;

/**
 *	data for the customization system, NOT THINGS THAT CAN BE CUSTOMIZED
 */

USTRUCT(BlueprintType)
struct FCustomizationModeCamSettings
{
	//the settings of the camera/camera boom for a mode
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RotateSpeed = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ZoomSpeed = 20.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MinZoom = 50.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxZoom = 1500.f;
};

UCLASS(BlueprintType)
class BATTLESERIES2_API UDA_CustomizationDefaults : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UUW_Customization> CustomizationWidgetClass = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stage Settings")
	TSubclassOf<ALoadoutPreviewStage> PreviewStageClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stage Settings")
	FTransform PreviewSpawnTransform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vehicle Settings")
	TSubclassOf<AVehicle_Base> PreviewVehicleClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vehicle Settings")
	TArray<EVehicleType> CustomizableVehicleTypes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<ECoreType, FCustomizationModeCamSettings> CustomizationModeCamSettings;
};
