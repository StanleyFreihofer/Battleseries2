#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "VehicleDefaults.generated.h"

enum class EVehicleType : uint8;

//defines traits of a vehicle type

USTRUCT(BlueprintType)
struct FVehicleTypeDefintion
{
	GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FText DisplayName = FText::GetEmpty();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FText Description = FText::GetEmpty();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TSoftObjectPtr<UTexture2D> TypeIcon = nullptr;               //minimap?

    //default seat count?
    //default turrets, weapons, etc?
};

UCLASS(BlueprintType)
class BATTLESERIES2_API UDA_VehicleDefaults : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSoftObjectPtr<UMaterialInterface> HUDMasterMaterial = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TMap<EVehicleType, FVehicleTypeDefintion> VehicleTypeDefintions;
};