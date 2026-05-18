// Fill out your copyright notice in the Description page of Project Settings.

#include "Save/SaveSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Save/PlayerSave_Loadout.h"
#include "Utilities/DataManagerSubsystem.h"
#include "Utilities/BS2FunctionLibrary.h"

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
	for (auto& Pair : LoadoutSave->VehicleLoadoutConfigs)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SaveSubsystem::Initialize] Loaded VehicleConfig key: %d, Seats: %d"), (int32)Pair.Key, Pair.Value.SeatLoadout.Num());
	}

}

void USaveSubsystem::Deinitialize()
{
	LoadoutSave = nullptr;
}

const FPlayerLoadoutConfig_Weapon& USaveSubsystem::GetWeaponLoadout(FName WeaponID)
{
	if (const FPlayerLoadoutConfig_Weapon* WeaponLoadout = LoadoutSave->WeaponConfigs.Find(WeaponID))
	{
		return *WeaponLoadout;
	}
	static FPlayerLoadoutConfig_Weapon Empty;
	return Empty;
}

const FPlayerLoadoutConfig_Class& USaveSubsystem::GetClassLoadout(EClassType ClassType)
{
	if (const FPlayerLoadoutConfig_Class* ClassLoadout = LoadoutSave->ClassLoadoutConfigs.Find(ClassType))
	{
		return *ClassLoadout;
	}
	static FPlayerLoadoutConfig_Class Empty;
	return Empty;
}

const FPlayerLoadoutConfig_Vehicle& USaveSubsystem::GetVehicleLoadout(EVehicleType VehicleType)
{
	if (const FPlayerLoadoutConfig_Vehicle* LoadoutForVehicle = LoadoutSave->VehicleLoadoutConfigs.Find(VehicleType))
	{
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
	FSavedSeatLoadout* Result = LoadoutSave->VehicleLoadoutConfigs[VehicleType].SeatLoadout.Find(SeatIndex);
	checkf(Result, TEXT("[SaveSubsystem::GetSeatLoadout] Failed to find/create SeatLoadout for index %d"), SeatIndex);
	return *Result;
}

void USaveSubsystem::SetLoadoutWeaponChoice_Infantry(EClassType ClassType, int32 WeaponIndex, FName WeaponID)
{
	FPlayerLoadoutConfig_Class& ClassConfig = LoadoutSave->ClassLoadoutConfigs.FindOrAdd(ClassType);
	if (WeaponIndex >= ClassConfig.Weapons.Num())
	{
		ClassConfig.Weapons.SetNum(WeaponIndex + 1);
	}
	ClassConfig.Weapons[WeaponIndex] = WeaponID;
	UGameplayStatics::SaveGameToSlot(LoadoutSave, "PlayerLoadouts", 0);
}

void USaveSubsystem::SetLoadoutWeaponChoice_Vehicle(EVehicleType VehicleType, int32 SeatIndex, int32 WeaponIndex, FName WeaponID)
{
	FPlayerLoadoutConfig_Vehicle& VehicleConfig = LoadoutSave->VehicleLoadoutConfigs.FindOrAdd(VehicleType);
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
	for (auto& Pair : LoadoutSave->VehicleLoadoutConfigs)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SaveSubsystem::SetLoadoutWeaponChoice_Vehicle] Saving VehicleConfig key: %d, Seats: %d"), (int32)Pair.Key, Pair.Value.SeatLoadout.Num());
	}

	UGameplayStatics::SaveGameToSlot(LoadoutSave, "PlayerLoadouts", 0);
	UE_LOG(LogTemp, Warning, TEXT("[SaveSubsystem::SetLoadoutWeaponChoice_Vehicle] Saved Vehicle %d Seat %d Slot %d = %s"), (int32)VehicleType, SeatIndex, WeaponIndex, *WeaponID.ToString());
}

void USaveSubsystem::SetLoadoutOpticChoice_Vehicle(EVehicleType VehicleType, int32 SeatIndex, FName OpticID)
{
	FPlayerLoadoutConfig_Vehicle& VehicleConfig = LoadoutSave->VehicleLoadoutConfigs.FindOrAdd(VehicleType);
	FSavedSeatLoadout& SeatConfig = VehicleConfig.SeatLoadout.FindOrAdd(SeatIndex);
	SeatConfig.Optic = OpticID;
	UGameplayStatics::SaveGameToSlot(LoadoutSave, "PlayerLoadouts", 0);
}

void USaveSubsystem::SetLoadoutCamoChoice_Vehicle(EVehicleType VehicleType, FName CamoID)
{
	FPlayerLoadoutConfig_Vehicle& VehicleConfig = LoadoutSave->VehicleLoadoutConfigs.FindOrAdd(VehicleType);
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
	FPlayerLoadoutConfig_Vehicle& VehicleConfig = LoadoutSave->VehicleLoadoutConfigs.FindOrAdd(VehicleType);
	FSavedSeatLoadout& SeatConfig = VehicleConfig.SeatLoadout.FindOrAdd(SeatIndex);
	SeatConfig.Weapons = Weapons;

	// Write to disk once
	UGameplayStatics::SaveGameToSlot(LoadoutSave, "PlayerLoadouts", 0);

	UE_LOG(LogTemp, Warning, TEXT("[SaveSubsystem::EnsureSeatDefaults] Saved default loadout for Vehicle %d Seat %d"), (int32)VehicleType, SeatIndex);
}

