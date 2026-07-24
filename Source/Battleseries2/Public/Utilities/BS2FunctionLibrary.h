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
    static bool PerformSphereTraceMulti(const UObject* WorldContextObject, const FTransform StartTransform, TArray<FHitResult>& OutHits, TArray<AActor*> ActorsToIgnore, float Radius, float Distance, bool Debug);
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

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Battleseries | Weapon Functions")
    static float GetFireRate(float RateOfFire);
    UFUNCTION(BlueprintCallable, Category = "Battleseries | Weapon Functions")
    static bool PerformWeaponLineTrace(const UObject* WorldContextObject, const FTransform& StartTransform, FHitResult& OutHit, TArray<AActor*> ActorsToIgnore, bool Debug);
    UFUNCTION(BlueprintCallable, Category = "Battleseries | Weapon Functions")
    static bool PerformWeaponSphereTrace(const UObject* WorldContextObject, const FTransform& StartTransform, FHitResult& OutHit, TArray<AActor*> ActorsToIgnore, float Radius, bool Debug);
    UFUNCTION()
    static FTransform GetMuzzleTransform(FName MuzzleSocketName, TWeakObjectPtr<USkeletalMeshComponent> SocketMesh);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Battleseries | Weapon Functions")
    static FVector CalculateAimDirection(FHitResult TraceData, FVector MuzzleLocation);
    UFUNCTION()
    static FVector GetAimDirectionFromMuzzle(FHitResult TraceData, FName MuzzleSocketName, TWeakObjectPtr<USkeletalMeshComponent> WeaponMesh);
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Battleseries | Weapon Functions")
	static FVector GetAimDirectionFromMuzzle_BP(FHitResult TraceData, FName MuzzleSocketName, USkeletalMeshComponent* WeaponMesh);
    UFUNCTION(BlueprintCallable, Category = "Battleseries | Weapon Functions")
    static FSimProjectile_Runtime CreateSimProjectile(FName MunitionID, class APlayerState* InstigatorPlayerState, FVector MuzzleLocation, float MuzzleSpeed, float GravityScale, FVector AimDirection, float BaseDamage, UCurveFloat* DamageDropoffCurve, UProjectilePoolSubsystem* ProjectileSubsystem);
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Battleseries | Weapon Functions")
    static void CalculateReload(int32 MagSize, int32 CAM, int32 CRA, int32& OutCAM, int32& OutCRA);
    UFUNCTION(BlueprintCallable, Category = "Battleseries | Weapon Functions")
    static int32 UpdateCurrentAmmoInMag(FWeapon_Runtime& CurrentWeapon, int32 AmmoDelta, int32 MagSize);
    UFUNCTION(BlueprintCallable, Category = "Battleseries | Weapon Functions")
    static void UpdateWeaponIndex(TArray<FWeapon_Runtime> Weapons, int32 InCurrentWeaponIndex, int32& OutNewWeaponIndex);

	UFUNCTION(BlueprintCallable, Category = "Battleseries | Optic Functions")
    static void HandleUpdateOptic(float inDefaultFOV, float inOpticMagnfication, float& OutOpticFOV, FPostProcessSettings inPostProcessData, FPostProcessSettings& OutPostProcessSettings, float& OutPostProcessWeight);
    UFUNCTION(BlueprintCallable, Category = "Battleseries | Optic Functions")
    static void UpdateOpticIndex(int32 TotalOptics, int32& CurrentOpticIndex);
};
