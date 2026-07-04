#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Data_Customization.generated.h"

enum class ECoreType : uint8;
enum class EVehicleType : uint8;
class AVehicle_Base;
class UUW_Customization;
class UUW_LoadoutSlot;
class UUW_DropdownOption;
class UUW_LoadoutTypeButton;
class ALoadoutPreviewStage;

/**
 *	data for the customization system, NOT THINGS THAT CAN BE CUSTOMIZED
 */

USTRUCT(BlueprintType)
struct FCustomizationMode_CamSettings
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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Settings")
	TSubclassOf<UUW_Customization> CustomizationWidgetClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Settings")
	TSubclassOf<UUW_LoadoutSlot> LoadoutSlotWidgetClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Settings")
	TSubclassOf<UUW_DropdownOption> DropdownOptionWidgetClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Settings")
	TSubclassOf<UUW_LoadoutTypeButton> LoadoutTypeWidgetClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Settings | Soldier Class Mode")
	int32 NumOfViewableTypeButtons_SoldierClassMode = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Settings | Soldier Class Mode")
	bool Scroll_SoldierClassMode = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Settings | Vehicle Mode")
	int32 NumOfViewableTypeButtons_VehicleMode = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Settings | Vehicle Mode")
	bool Scroll_VehicleMode = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stage Settings")
	TSubclassOf<ALoadoutPreviewStage> PreviewStageClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stage Settings")
	FTransform PreviewSpawnTransform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vehicle Settings")
	TSubclassOf<AVehicle_Base> PreviewVehicleClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vehicle Settings")
	TArray<EVehicleType> CustomizableVehicleTypes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<ECoreType, FCustomizationMode_CamSettings> CustomizationModeCamSettings;
};
