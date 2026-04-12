#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Vehicle_Base.h"
#include "Data_Customization.generated.h"

class UUW_Customization;
class ALoadoutPreviewStage;

/**
 *	data for the customization system, NOT THINGS THAT CAN BE CUSTOMIZED
 */

USTRUCT(BlueprintType)
struct CustomizationModeCamSettings
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<ALoadoutPreviewStage> PreviewStageClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AVehicle_Base> PreviewVehicleClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FTransform PreviewSpawnTransform = FTransform::Identity;

	//what vehicle types can be customized?
};
