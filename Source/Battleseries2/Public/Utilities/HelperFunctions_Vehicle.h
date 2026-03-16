#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DataTable.h"
#include "Data/Vehicles/Data_Vehicle.h"
#include "HelperFunctions_Vehicle.generated.h"

UCLASS()
class BATTLESERIES2_API UHelperFunctions_Vehicle : public UObject
{
    GENERATED_BODY()

public:
    // Convert all row names in a DataTable to enum values
    UFUNCTION(BlueprintCallable, Category = "Vehicle|HUD")
    static void ConvertNamesToVehicleTypes(const TArray<FName>& VehicleTypeNames, TArray<E_VehicleType>& OutVehicleTypes);

    // Convert enum value to its literal string name (not DisplayName)
    UFUNCTION(BlueprintCallable, Category = "Vehicle|HUD")
    static FString GetVehicleTypeLiteralString(E_VehicleType VehicleType);
};
