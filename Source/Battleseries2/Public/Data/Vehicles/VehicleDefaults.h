#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "VehicleDefaults.generated.h"

enum class E_VehicleType : uint8;

//defines traits of a vehicle type

USTRUCT(BlueprintType)
struct FVehicleTypeDefintion
{
	GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    E_VehicleType VehicleType;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FText DisplayName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FText Description;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TSoftObjectPtr<UTexture2D> TypeIcon;                //minimap?

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
    TMap<E_VehicleType, FVehicleTypeDefintion> VehicleTypeDefintions;
};