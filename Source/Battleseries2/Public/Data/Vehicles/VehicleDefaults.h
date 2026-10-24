#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "NiagaraSystem.h"
#include "InputMappingContext.h"
#include "Data/Core/Combat/Data_Combat.h"
#include "Data/Vehicles/VehicleEnums.h"
#include "VehicleDefaults.generated.h"

enum class EVehicleType : uint8;
enum class E_MovementType : uint8;
enum class E_SeatRole : uint8;

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
struct FSeatWeaponSlot
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText WeaponSlotDisplayName = FText::GetEmpty();
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FName> WeaponOptions;
};

USTRUCT(BlueprintType)
struct FSeatLoadoutOptions
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FSeatWeaponSlot> AvailableWeaponSlots;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FName> AvailableOptics = { "O_60hz"};
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FName> AvailableCountermeasures;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FName> AvailableUpgrades;
    
    TArray<FName> GetDefaultSeatWeapons() const 
    {
        TArray<FName> DefaultSeatWeapons;
        for (const FSeatWeaponSlot& WeaponSlot : AvailableWeaponSlots)
        {
            if (WeaponSlot.WeaponOptions.IsEmpty()) { continue; }
            DefaultSeatWeapons.Add(WeaponSlot.WeaponOptions[0]);
        }
        return DefaultSeatWeapons;
    }
    
    int32 GetNumWeaponSlots() const
    {
        return AvailableWeaponSlots.Num();
    }
};

USTRUCT(BlueprintType)
struct FVehicleTypeSeatDefinition
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (tooltip = "also used for the attaching of player character to socket on vehicle mesh"))
    FText SeatName = FText::GetEmpty();

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    E_SeatRole SeatRole = E_SeatRole::Driver;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TObjectPtr<UInputMappingContext> InputMappingContext = nullptr;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (tooltip = "The Array Indexes of the turret(s) this seat controls"))
    TArray<int32> ControlledTurretIndexes;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FSeatLoadoutOptions SeatLoadoutOptions = FSeatLoadoutOptions();
};

USTRUCT(BlueprintType)
struct FVehicleTypeDefinition        //entity type definition (store in core instead?)
{
	GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FText DisplayName = FText::GetEmpty();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FText Description = FText::GetEmpty();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TSoftObjectPtr<UTexture2D> TypeIcon = nullptr;               //minimap?
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    E_MovementType Movement_Type = E_MovementType::GroundVehicle;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FVehicleTypeSeatDefinition> SeatDefinitions;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FVehicleTypeHealthDefinition VehicleHealthDefinition = FVehicleTypeHealthDefinition();
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FVehicleCombatDefinition VehicleCombatDefinition = FVehicleCombatDefinition();
    
    int32 GetNumSeats() const
    {
        return SeatDefinitions.Num();
    }
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
    TMap<EVehicleType, FVehicleTypeDefinition> VehicleTypeDefintions;
};