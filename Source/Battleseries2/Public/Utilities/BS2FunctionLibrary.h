#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DataTable.h"
#include "Data/Vehicles/Data_Vehicle.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BS2FunctionLibrary.generated.h"

class UDataManagerSubsystem;
class UHUDSubsystem;
class USaveSubsystem;
class UProjectilePoolSubsystem;
class IVehicleDataAccessor;

UCLASS()
class BATTLESERIES2_API UBS2FunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintCallable, Category = "Battleseries | Tools")
    static bool PerformSphereTraceMulti(const UObject* WorldContextObject, const FTransform StartTransform, TArray<FHitResult>& OutHits, TArray<AActor*> ActorsToIgnore, float Radius, float Distance);
    UFUNCTION(BlueprintCallable, Category = "Vehicle|HUD")
    static void ConvertNamesToVehicleTypes(const TArray<FName>& VehicleTypeNames, TArray<EVehicleType>& OutVehicleTypes);

    // Convert enum value to its literal string name (not DisplayName)
    UFUNCTION(BlueprintCallable, Category = "Vehicle|HUD")
    static FString GetVehicleTypeLiteralString(EVehicleType VehicleType);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Battleseries | Subsystems")
    static UDataManagerSubsystem* GetDataSubsystem(const UObject* WorldContextObject);
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Battleseries | Subsystems")
    static UHUDSubsystem* GetHUDSubsystem(const UObject* WorldContextObject);
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Battleseries | Subsystems")
    static USaveSubsystem* GetSaveSubsystem(const UObject* WorldContextObject);
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Battleseries | Subsystems")
    static UProjectilePoolSubsystem* GetProjectileSystem(const UObject* WorldContextObject);
    static IVehicleDataAccessor* GetVehicleAccessor(AActor* TargetActor);
};
