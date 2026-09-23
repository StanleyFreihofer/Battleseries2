#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "NiagaraSystem.h"
#include "Data/Core/Combat/Data_Combat.h"
#include "VehicleDefaults.generated.h"

enum class EVehicleType : uint8;

//defines traits of a vehicle type

USTRUCT(BlueprintType)
struct FVehicleTypeHealthDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FHealthData BaseHealthData = FHealthData();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "% of max health dealt in a single hit to cause a mobility hit (movement impacted but not disabled)"))
    float MobilityHitThreshold = 0.30f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "% of max health dealt in a single hit to cause a mobility kill (movement fully disabled)"))
    float MobilityKillThreshold = 0.40f;
};

USTRUCT(BlueprintType)
struct FVehicleCombatDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "hits within this many degrees of straight-down onto the vehicle count as top armor"))
    float TopArmorAngle = 30.f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TMap<FName, float> ArmorHitMultipliers = 
    {
        {"FrontArmor", 0.75f},
        {"RearArmor", 2.0f},
        {"SideArmor", 1.0f},
        {"TopArmor", 1.25f}
    };
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<TSoftObjectPtr<UNiagaraSystem>> InitialFireballFX;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<TSoftObjectPtr<UNiagaraSystem>> ExplosionFX;
};

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
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FVehicleTypeHealthDefinition VehicleHealthDefinition = FVehicleTypeHealthDefinition();
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FVehicleCombatDefinition VehicleCombatDefinition = FVehicleCombatDefinition();

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
    TArray<TSoftObjectPtr<UNiagaraSystem>> FireFX;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TMap<EVehicleType, FVehicleTypeDefintion> VehicleTypeDefintions;
};