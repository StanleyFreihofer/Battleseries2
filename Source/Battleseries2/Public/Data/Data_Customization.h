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

UCLASS(BlueprintType)
class BATTLESERIES2_API UDA_CustomizationDefaults : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UUW_Customization> CustomizationWidgetClass = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<ALoadoutPreviewStage> PreviewStageClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AVehicle_Base> PreviewVehicleClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* GarageMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform PreviewSpawnTransform = FTransform::Identity;

	//what vehicle types can be customized?
};
