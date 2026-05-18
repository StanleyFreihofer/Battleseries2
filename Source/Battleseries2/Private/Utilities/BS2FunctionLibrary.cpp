#include "Utilities/BS2FunctionLibrary.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Utilities/BS2FunctionLibrary.h"
#include "Utilities/DataManagerSubsystem.h"
#include "Utilities/ProjectilePoolSubsystem.h"
#include "Utilities/HUDSubsystem.h"
#include "Save/SaveSubsystem.h"
#include "Utilities/I_VehicleDataAccessor.h"


void UBS2FunctionLibrary::ConvertNamesToVehicleTypes(const TArray<FName>& VehicleTypeNames, TArray<EVehicleType>& OutVehicleTypes)
{
	OutVehicleTypes.Empty();
	for (const FName& VehicleTypeName : VehicleTypeNames)
	{
		//FString VehicleTypeString = RowName.ToString();

		//EVehicleType VehicleType = EVehicleType::VE_None; //default/fallback
		
		int64 FoundEnum = StaticEnum<EVehicleType>()->GetValueByName(VehicleTypeName);
		if (FoundEnum != INDEX_NONE)
		{
			OutVehicleTypes.Add(static_cast<EVehicleType>(FoundEnum));
			//return FoundEnum;
		}
		
	}
}

FString UBS2FunctionLibrary::GetVehicleTypeLiteralString(EVehicleType VehicleType)
{
	// Use StaticEnum to look up the name
	UEnum* EnumPtr = StaticEnum<EVehicleType>();
	if (!EnumPtr)
	{
		return FString("Invalid");
	}

	// Get the literal enum name (e.g. "IFV", "Tank", etc.)
	return EnumPtr->GetNameStringByValue(static_cast<int64>(VehicleType));
}

UDataManagerSubsystem* UBS2FunctionLibrary::GetDataSubsystem(const UObject* WorldContextObject)
{
	return WorldContextObject->GetWorld()->GetGameInstance()->GetSubsystem<UDataManagerSubsystem>();
}

UHUDSubsystem* UBS2FunctionLibrary::GetHUDSubsystem(const UObject* WorldContextObject)
{
	return WorldContextObject->GetWorld()->GetFirstLocalPlayerFromController()->GetSubsystem<UHUDSubsystem>();
}

USaveSubsystem* UBS2FunctionLibrary::GetSaveSubsystem(const UObject* WorldContextObject)
{
	return WorldContextObject->GetWorld()->GetGameInstance()->GetSubsystem<USaveSubsystem>();
}

UProjectilePoolSubsystem* UBS2FunctionLibrary::GetProjectileSystem(const UObject* WorldContextObject)
{
	return WorldContextObject->GetWorld()->GetSubsystem<UProjectilePoolSubsystem>();
}

IVehicleDataAccessor* UBS2FunctionLibrary::GetVehicleAccessor(AActor* TargetActor)
{
	return Cast<IVehicleDataAccessor>(TargetActor);
}



