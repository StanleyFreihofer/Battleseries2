// Fill out your copyright notice in the Description page of Project Settings.

#include "Save/SaveSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Save/PlayerSave_Loadout.h"
#include "Utilities/DataManagerSubsystem.h"
#include "Utilities/HelperFunctions_Vehicle.h"

void USaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FString SlotName = TEXT("PlayerLoadouts");
	int32 UserIndex = 0;

	if (UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex))
	{
		LoadoutSave = Cast<UPlayerSave_Loadout>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));
	}
	else
	{
		LoadoutSave = Cast<UPlayerSave_Loadout>(UGameplayStatics::CreateSaveGameObject(UPlayerSave_Loadout::StaticClass()));
		UGameplayStatics::SaveGameToSlot(LoadoutSave, SlotName, UserIndex);
	}
	for (auto& Pair : LoadoutSave->VehicleConfigs)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SaveSubsystem::Initialize] Loaded VehicleConfig key: %d, Seats: %d"), (int32)Pair.Key, Pair.Value.SeatLoadout.Num());
	}

}

void USaveSubsystem::Deinitialize()
{
	LoadoutSave = nullptr;
}

const FPlayerLoadoutConfig_Vehicle& USaveSubsystem::GetVehicleLoadout(EVehicleType VehicleType)
{
	if (const FPlayerLoadoutConfig_Vehicle* LoadoutForVehicle = LoadoutSave->VehicleConfigs.Find(VehicleType))
	{
		UE_LOG(LogTemp, Warning, TEXT("[SaveSubsystem::GetVehicleLoadout] unknown crash here"));
		//UE_LOG(LogTemp, Warning, TEXT("GetVehicleLoadout, Returning LoadoutForVehicle: %s"), *LoadoutSave->VehicleConfigs.Find(VehicleType)->SeatLoadout.Find(0)->Weapons[0].ToString());
		return *LoadoutForVehicle;
	}
	UE_LOG(LogTemp, Warning, TEXT("[SaveSubsystem::GetVehicleLoadout] returning empty loadout"));
	static FPlayerLoadoutConfig_Vehicle Empty;
	return Empty;
}

const FSavedSeatLoadout& USaveSubsystem::GetSeatLoadout(EVehicleType VehicleType, int32 SeatIndex)
{
	const FPlayerLoadoutConfig_Vehicle& VehicleLoadout = GetVehicleLoadout(VehicleType);
	if (!VehicleLoadout.SeatLoadout.Contains(SeatIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("[SaveSubsystem::GetSeatLoadout] SeatIndex %d not found, populating defaults..."), SeatIndex);
		EnsureSeatDefaults(VehicleType, SeatIndex);
		// Note: EnsureSeatDefaults might have reallocated the map memory
	}
	// auto-populate and then re-fetch
	FSavedSeatLoadout* Result = LoadoutSave->VehicleConfigs[VehicleType].SeatLoadout.Find(SeatIndex);
	checkf(Result, TEXT("[SaveSubsystem::GetSeatLoadout] Failed to find/create SeatLoadout for index %d"), SeatIndex);
	return *Result;
}

void USaveSubsystem::SetLoadoutWeaponChoice_Vehicle(EVehicleType VehicleType, int32 SeatIndex, int32 WeaponIndex, FName WeaponID)
{
	FPlayerLoadoutConfig_Vehicle& VehicleConfig = LoadoutSave->VehicleConfigs.FindOrAdd(VehicleType);
	FSavedSeatLoadout& SeatConfig = VehicleConfig.SeatLoadout.FindOrAdd(SeatIndex);

	//Ensure all previous weapon slots exist (even if None)
	if (SeatConfig.Weapons.Num() <= WeaponIndex)
	{
		const int32 OldNum = SeatConfig.Weapons.Num();
		SeatConfig.Weapons.SetNum(WeaponIndex + 1);
		for (int32 i = OldNum; i < SeatConfig.Weapons.Num(); ++i)
		{
			if (SeatConfig.Weapons[i].IsNone())
			{
				SeatConfig.Weapons[i] = NAME_None;
			}
		}
	}

	SeatConfig.Weapons[WeaponIndex] = WeaponID;

	UE_LOG(LogTemp, Warning, TEXT("[SaveSubsystem::SetLoadoutWeaponChoice_Vehicle] Saved Vehicle %d Seat %d Slot %d = %s"), (int32)VehicleType, SeatIndex, WeaponIndex, *WeaponID.ToString());
	for (auto& Pair : LoadoutSave->VehicleConfigs)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SaveSubsystem::SetLoadoutWeaponChoice_Vehicle] Saving VehicleConfig key: %d, Seats: %d"), (int32)Pair.Key, Pair.Value.SeatLoadout.Num());
	}

	UGameplayStatics::SaveGameToSlot(LoadoutSave, "PlayerLoadouts", 0);
	UE_LOG(LogTemp, Warning, TEXT("[SaveSubsystem::SetLoadoutWeaponChoice_Vehicle] Saved Vehicle %d Seat %d Slot %d = %s"), (int32)VehicleType, SeatIndex, WeaponIndex, *WeaponID.ToString());
}

void USaveSubsystem::SetLoadoutOpticChoice_Vehicle(EVehicleType VehicleType, int32 SeatIndex, FName OpticID)
{
	FPlayerLoadoutConfig_Vehicle& VehicleConfig = LoadoutSave->VehicleConfigs.FindOrAdd(VehicleType);
	FSavedSeatLoadout& SeatConfig = VehicleConfig.SeatLoadout.FindOrAdd(SeatIndex);
	SeatConfig.Optic = OpticID;
	UGameplayStatics::SaveGameToSlot(LoadoutSave, "PlayerLoadouts", 0);
}

void USaveSubsystem::SetLoadoutCamoChoice_Vehicle(EVehicleType VehicleType, FName CamoID)
{
	FPlayerLoadoutConfig_Vehicle& VehicleConfig = LoadoutSave->VehicleConfigs.FindOrAdd(VehicleType);
	VehicleConfig.VehicleCamo = CamoID;
	UGameplayStatics::SaveGameToSlot(LoadoutSave, "PlayerLoadouts", 0);
}

void USaveSubsystem::EnsureSeatDefaults(EVehicleType VehicleType, int32 SeatIndex)
{
	UDataManagerSubsystem* DataSubsystem = GetGameInstance()->GetSubsystem<UDataManagerSubsystem>();
	UDataTable* VehicleDataTable = DataSubsystem->GetVehicleDataTable();
	TArray<FName> AllVehicleIDsOfType = FVehicleData::GetRowNamesOfType(VehicleDataTable, VehicleType);
	const FVehicleData* VehicleData = DataSubsystem->GetVehicleDataRow(AllVehicleIDsOfType[0]);
	const FSeatData* SeatData = &VehicleData->Seats[SeatIndex];
	int32 NumOfWeaponSlots = SeatData->AvailableItems.AvailableWeaponSlots.Num();
	TArray<FName> Weapons;
	Weapons.SetNum(NumOfWeaponSlots);

	for (int32 i = 0; i < NumOfWeaponSlots; i++)
	{
		const TMap<FName, FVehicleWeaponInstanceData>& WeaponChoiceMap = SeatData->AvailableItems.AvailableWeaponSlots[i].WeaponChoices;
		auto It = WeaponChoiceMap.CreateConstIterator();
		Weapons[i] = It.Key();
	}

	// Apply to the save
	FPlayerLoadoutConfig_Vehicle& VehicleConfig = LoadoutSave->VehicleConfigs.FindOrAdd(VehicleType);
	FSavedSeatLoadout& SeatConfig = VehicleConfig.SeatLoadout.FindOrAdd(SeatIndex);
	SeatConfig.Weapons = Weapons;

	// Write to disk once
	UGameplayStatics::SaveGameToSlot(LoadoutSave, "PlayerLoadouts", 0);

	UE_LOG(LogTemp, Warning, TEXT("[SaveSubsystem::EnsureSeatDefaults] Saved default loadout for Vehicle %d Seat %d"), (int32)VehicleType, SeatIndex);
}

