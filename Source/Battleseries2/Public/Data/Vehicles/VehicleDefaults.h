#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "VehicleDefaults.generated.h"

enum class EVehicleType : uint8;

//defines traits of a vehicle type

USTRUCT(BlueprintType)
struct FVehicleTypeDefintion        //entity type definition (store in core instead?)
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

USTRUCT(BlueprintType)
struct FVehicleCombatDefinition
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "hits within this many degrees of forward count as front armor"))
    float FrontArmorAngle = 45.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "hits within this many degrees of directly behind count as rear armor"))
    float RearArmorAngle = 45.f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float SideArmorAngle = 45.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "hits within this many degrees of straight-down onto the vehicle count as top armor"))
    float TopArmorAngle = 30.f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TMap<FName, float> ArmorHitMultipliers = 
    {
        {"FrontArmor", 0.75f},
        {"RearArmor", 1.0f},
        {"SideArmor", 2.0f},
        {"TopArmor", 1.25f}
    };
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
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FVehicleCombatDefinition VehicleCombatDefinition = FVehicleCombatDefinition();
};