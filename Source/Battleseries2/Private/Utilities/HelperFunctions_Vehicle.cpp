#include "Utilities/HelperFunctions_Vehicle.h"

void UHelperFunctions_Vehicle::ConvertNamesToVehicleTypes(const TArray<FName>& VehicleTypeNames, TArray<EVehicleType>& OutVehicleTypes)
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

FString UHelperFunctions_Vehicle::GetVehicleTypeLiteralString(EVehicleType VehicleType)
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



