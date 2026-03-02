#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Engine/DataAsset.h"
#include "DataManagerSubsystem.generated.h"

struct FVehicleData;
struct FVehicleTypeDefintion;
struct FVehicleWeaponData;
struct FAttachmentData;
struct FProjectileData;
struct FCamoData;
enum class E_VehicleType : uint8;
class UDA_VehicleDefaults;
class UDA_WeaponDefaults;
class UDA_CoreTypes;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDataReady);

UCLASS(Blueprintable, BlueprintType)
class BATTLESERIES2_API UDataManagerSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // Editor-assigned soft references
    UPROPERTY(EditAnywhere, Category = "Data Tables")
    TSoftObjectPtr<UDataTable> VehicleDataTableAsset;

    UPROPERTY(EditAnywhere, Category = "Data Tables")
    TSoftObjectPtr<UDataTable> VehicleWeaponDataTableAsset;

    UPROPERTY(EditAnywhere, Category = "Data Tables")
    TSoftObjectPtr<UDataTable> ProjectileDataTableAsset;

    UPROPERTY(EditAnywhere, Category = "Data Tables")
    TSoftObjectPtr<UDataTable> AttachmentDataTableAsset;

    UPROPERTY(EditAnywhere, Category = "Data Tables")
    TSoftObjectPtr<UDataTable> CamoDataTableAsset;

    UPROPERTY(EditAnywhere, Category = "Data Assets")
    TSoftObjectPtr<UDA_WeaponDefaults> WeaponDefaultsDAAsset;

    UPROPERTY(EditAnywhere, Category = "Data Assets")
    TSoftObjectPtr<UDA_VehicleDefaults> VehicleDefaultsDAAsset;

    UPROPERTY(EditAnywhere, Category = "Data Assets")
    TSoftObjectPtr<UDA_CoreTypes> CoreTypeDefinitionsDAAsset;




    UPROPERTY(BlueprintAssignable)
    FOnDataReady OnDataReady;


    UDataManagerSubsystem();    //constructor
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    UFUNCTION(BlueprintCallable)
    void LoadDataTables();

    //HELPER FUNCTIONS

    UFUNCTION(BlueprintCallable, Category = "Data")
    UDataTable* GetVehicleDataTable() const { return VehicleDataTable; }

    UFUNCTION(BlueprintCallable, Category = "Data")
    UDataTable* GetVehicleWeaponDataTable() const { return VehicleWeaponDataTable; }

    UFUNCTION(BlueprintCallable, Category = "Data")
    UDataTable* GetProjectileDataTable() const { return ProjectileDataTable; }

    UFUNCTION(BlueprintCallable, Category = "Data")
    UDataTable* GetAttachmentDataTable() const { return AttachmentDataTable; }

    UFUNCTION(BlueprintCallable, Category = "Data")
    UDataTable* GetWeaponDataTable() const { return WeaponDataTable; }

    UFUNCTION(BlueprintCallable, Category = "Data")
    UDA_CoreTypes* GetCoreTypeDefinitions() const { return CoreTypeDefinitionsDataAsset;  }

    UFUNCTION(BlueprintCallable, Category = "Data")
    UDA_WeaponDefaults* GetWeaponDefaults() const { return WeaponDefaultsDataAsset; }

    UFUNCTION(BlueprintCallable, Category = "Data")
    UDA_VehicleDefaults* GetVehicleDefaults() const { return VehicleDefaultsDataAsset; }


    //USTRUCTs are value types, not objects like UCLASS.
    //UFUNCTIONs exposed to Blueprints cannot return raw pointers or references to USTRUCTs.
    //Unreal doesn’t allow exposing memory addresses of value types to Blueprints.
    //in conclusion, another Blueprint L
    UFUNCTION(BlueprintCallable, Category = "Data")
    FVehicleData GetVehicleDataRowCopy(FName RowName) const;

    UFUNCTION(BlueprintCallable, Category = "Data")
    FVehicleWeaponData GetVehicleWeaponDataRowCopy(FName RowName) const;
    UFUNCTION(BlueprintCallable, Category = "Data")
    FAttachmentData GetAttachmentDataRowCopy(FName RowName) const;
    UFUNCTION(BlueprintCallable, Category = "Data")
    FProjectileData GetProjectileDataRowCopy(FName RowName) const;
    UFUNCTION(BlueprintCallable, Category = "Data")
    bool IsDataReady() const { return bDataReady; }
    UFUNCTION(BlueprintCallable)
    TArray<FName> GetAllVehicleIDs() const;
    UFUNCTION(BlueprintCallable)
    TArray<FName> GetAllProjectileIDs() const;
    UFUNCTION(BlueprintCallable)
    FName GetFirstVehicleIDOfType(E_VehicleType VehicleType) const;
    UFUNCTION(BlueprintCallable)
    FText GetWeaponSlotName(int32 WeaponSlotIndex);

    UFUNCTION(BlueprintCallable)
    void PreloadCoreAssets();


    //CPP Only Functions
    const FVehicleData* GetVehicleDataRow(FName RowName) const;
    const FVehicleWeaponData* GetVehicleWeaponDataRow(FName RowName) const;
    const FAttachmentData* GetAttachmentDataRow(FName RowName) const;
    const FProjectileData* GetProjectileDataRow(FName RowName) const;
    const FCamoData* GetCamoDataRow(FName RowName) const;

private:
    //loaded data
    UPROPERTY()
    UDataTable* VehicleDataTable;

    UPROPERTY()
    UDataTable* VehicleWeaponDataTable;

    UPROPERTY()
    UDataTable* ProjectileDataTable;

    UPROPERTY()
    UDataTable* WeaponDataTable;

    UPROPERTY()
    UDataTable* AttachmentDataTable;

    UPROPERTY()
    UDataTable* CamoDataTable;

    UPROPERTY()
    UDA_CoreTypes* CoreTypeDefinitionsDataAsset;

    UPROPERTY()
    UDA_WeaponDefaults* WeaponDefaultsDataAsset;

    UPROPERTY()
    UDA_VehicleDefaults* VehicleDefaultsDataAsset;


    bool bDataReady = false;

};