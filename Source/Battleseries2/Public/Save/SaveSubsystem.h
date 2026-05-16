// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/Vehicles/Data_Vehicle.h"
#include "SaveSubsystem.generated.h"

class UPlayerSave_Loadout;
enum class EClassType : uint8;
struct FPlayerLoadoutConfig_Weapon;
struct FPlayerLoadoutConfig_Class;

/**
 * 
 */
UCLASS()
class BATTLESERIES2_API USaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	public:
		virtual void Initialize(FSubsystemCollectionBase& Collection) override;
		virtual void Deinitialize() override;

		UPROPERTY(BlueprintReadOnly)
		class UPlayerSave_Loadout* LoadoutSave;
		UFUNCTION(BlueprintCallable)
		UPlayerSave_Loadout* GetLoadoutSave() const { return LoadoutSave; }
		UFUNCTION(BlueprintCallable)
		const FPlayerLoadoutConfig_Weapon& GetWeaponLoadout(FName WeaponID);
		UFUNCTION(BlueprintCallable)
		const FPlayerLoadoutConfig_Class& GetClassLoadout(EClassType ClassType);
		UFUNCTION(BlueprintCallable)
		const FPlayerLoadoutConfig_Vehicle& GetVehicleLoadout(EVehicleType VehicleType);
		UFUNCTION(BlueprintCallable)
		const FSavedSeatLoadout& GetSeatLoadout(EVehicleType VehicleType, int32 SeatIndex);

		UFUNCTION(BlueprintCallable)
		void SetLoadoutWeaponChoice_Infantry(EClassType ClassType, int32 WeaponIndex, FName WeaponID);
		UFUNCTION(BlueprintCallable)
		void SetLoadoutWeaponChoice_Vehicle(EVehicleType VehicleType, int32 SeatIndex, int32 WeaponIndex, FName WeaponID);
		UFUNCTION(BlueprintCallable)
		void SetLoadoutOpticChoice_Vehicle(EVehicleType VehicleType, int32 SeatIndex, FName OpticID);
		UFUNCTION(BlueprintCallable)
		void SetLoadoutCamoChoice_Vehicle(EVehicleType VehicleType, FName CamoID);

		UFUNCTION(BlueprintCallable)
		void EnsureSeatDefaults(EVehicleType VehicleType, int32 SeatIndex);
	
};
