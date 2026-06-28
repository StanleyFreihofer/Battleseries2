#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Engine/DataAsset.h"
#include "DataManagerSubsystem.generated.h"

struct FInfantryWeaponData;
struct FWeaponAttachmentData;
struct FVehicleData;
struct FVehicleTypeDefintion;
struct FVehicleWeaponData;
struct FVehicleAttachmentData;
struct FOpticData;
struct FProjectileData;
struct FCamoData;
enum class EWeaponType : uint8;
enum class EVehicleType : uint8;
class UDA_CharacterDefaults;
class UDA_VehicleDefaults;
class UDA_SoldierClassDefaults;
class UDA_WeaponDefaults;
class UDA_CustomizationDefaults;
class UDA_CoreDefaults;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDataReady);

UCLASS(Blueprintable, BlueprintType)
class BATTLESERIES2_API UDataManagerSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // Editor-assigned soft references
    UPROPERTY(EditAnywhere, Category = "Data Tables")
    TSoftObjectPtr<UDataTable> InfantryWeaponDataTableAsset;

    UPROPERTY(EditAnywhere, Category = "Data Tables")
    TSoftObjectPtr<UDataTable> WeaponAttachmentDataTableAsset;

    UPROPERTY(EditAnywhere, Category = "Data Tables")
    TSoftObjectPtr<UDataTable> VehicleDataTableAsset;

    UPROPERTY(EditAnywhere, Category = "Data Tables")
    TSoftObjectPtr<UDataTable> VehicleWeaponDataTableAsset;

    UPROPERTY(EditAnywhere, Category = "Data Tables")
    TSoftObjectPtr<UDataTable> ProjectileDataTableAsset;

    UPROPERTY(EditAnywhere, Category = "Data Tables")
    TSoftObjectPtr<UDataTable> VehicleAttachmentDataTableAsset;

    UPROPERTY(EditAnywhere, Category = "Data Tables")
    TSoftObjectPtr<UDataTable> OpticDataTableAsset;

    UPROPERTY(EditAnywhere, Category = "Data Tables")
    TSoftObjectPtr<UDataTable> CamoDataTableAsset;

    UPROPERTY(EditAnywhere, Category = "Data Assets")
    TSoftObjectPtr<UDA_CharacterDefaults> CharacterDefaultsDAAsset;

    UPROPERTY(EditAnywhere, Category = "Data Assets")
    TSoftObjectPtr<UDA_SoldierClassDefaults> SoldierClassDefaultsDAAsset;

    UPROPERTY(EditAnywhere, Category = "Data Assets")
    TSoftObjectPtr<UDA_WeaponDefaults> WeaponDefaultsDAAsset;

    UPROPERTY(EditAnywhere, Category = "Data Assets")
    TSoftObjectPtr<UDA_VehicleDefaults> VehicleDefaultsDAAsset;

    UPROPERTY(EditAnywhere, Category = "Data Assets")
    TSoftObjectPtr<UDA_CustomizationDefaults> CustomizationDefaultsDAAsset;

    UPROPERTY(EditAnywhere, Category = "Data Assets")
    TSoftObjectPtr<UDA_CoreDefaults> CoreDefaultsDefinitionsDAAsset;




    UPROPERTY(BlueprintAssignable)
    FOnDataReady OnDataReady;


    UDataManagerSubsystem();    //constructor
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    UFUNCTION(BlueprintCallable)
    void LoadDataTables();

    //HELPER FUNCTIONS

#pragma region DataTableGetters
    UFUNCTION(BlueprintCallable, Category = "Data")
    UDataTable* GetInfantryWeaponDataTable() const { return InfantryWeaponDataTable; }

    UFUNCTION(BlueprintCallable, Category = "Data")
    UDataTable* GetVehicleDataTable() const { return VehicleDataTable; }

    UFUNCTION(BlueprintCallable, Category = "Data")
    UDataTable* GetVehicleWeaponDataTable() const { return VehicleWeaponDataTable; }

    UFUNCTION(BlueprintCallable, Category = "Data")
    UDataTable* GetProjectileDataTable() const { return ProjectileDataTable; }

    UFUNCTION(BlueprintCallable, Category = "Data")
    UDataTable* GetVehicleAttachmentDataTable() const { return VehicleAttachmentDataTable; }

    UFUNCTION(BlueprintCallable, Category = "Data")
    UDataTable* GetOpticDataTable() const { return OpticDataTable; }

    UFUNCTION(BlueprintCallable, Category = "Data")
    UDataTable* GetWeaponDataTable() const { return WeaponDataTable; }

    UFUNCTION(BlueprintCallable, Category = "Data")
    UDA_CoreDefaults* GetCoreDefaultsDefinitions() const { return CoreDefaultsDefinitionsDataAsset;  }

    UFUNCTION(BlueprintCallable, Category = "Data")
    UDA_CharacterDefaults* GetCharacterDefaults() const { return CharacterDefaultsDataAsset; }

    UFUNCTION(BlueprintCallable, Category = "Data")
    UDA_SoldierClassDefaults* GetSoldierClassDefaults() const { return SoldierClassDefaultsDataAsset; }

    UFUNCTION(BlueprintCallable, Category = "Data")
    UDA_WeaponDefaults* GetWeaponDefaults() const { return WeaponDefaultsDataAsset; }

    UFUNCTION(BlueprintCallable, Category = "Data")
    UDA_VehicleDefaults* GetVehicleDefaults() const { return VehicleDefaultsDataAsset; }

    UFUNCTION(BlueprintCallable, Category = "Data")
    UDA_CustomizationDefaults* GetCustomizationDefaults() const { return CustomizationDefaultsDataAsset; }

#pragma endregion

#pragma region BlueprintCallableRowGetters
    //USTRUCTs are value types, not objects like UCLASS.
    //UFUNCTIONs exposed to Blueprints cannot return raw pointers or references to USTRUCTs.
    //Unreal doesn’t allow exposing memory addresses of value types to Blueprints.
    //in conclusion, another Blueprint L
    UFUNCTION(BlueprintCallable, Category = "Data")
    FVehicleData GetVehicleDataRowCopy(FName RowName) const;
    UFUNCTION(BlueprintCallable, Category = "Data")
    FVehicleWeaponData GetVehicleWeaponDataRowCopy(FName RowName) const;
    UFUNCTION(BlueprintCallable, Category = "Data")
    FVehicleAttachmentData GetVehicleAttachmentDataRowCopy(FName RowName) const;
    UFUNCTION(BlueprintCallable, Category = "Data")
    FInfantryWeaponData GetInfantryWeaponDataRowCopy(FName RowName) const;
    UFUNCTION(BlueprintCallable, Category = "Data")
    FWeaponAttachmentData GetWeaponAttachmentDataRowCopy(FName RowName) const;
    UFUNCTION(BlueprintCallable, Category = "Data")
    FProjectileData GetProjectileDataRowCopy(FName RowName) const;
    UFUNCTION(BlueprintCallable, Category = "Data")
    bool IsDataReady() const { return bDataReady; }
    UFUNCTION(BlueprintCallable)
    TArray<FName> GetAllInfantryWeaponIDs() const;
    UFUNCTION(BlueprintCallable)
    TArray<FName> GetAllVehicleIDs() const;
    UFUNCTION(BlueprintCallable)
    TArray<FName> GetAllProjectileIDs() const;
    UFUNCTION(BlueprintCallable)
    FName GetFirstVehicleIDOfType(EVehicleType VehicleType) const;
    UFUNCTION(BlueprintCallable)
    FText GetWeaponSlotName(int32 WeaponSlotIndex);
    UFUNCTION(BlueprintCallable)
    TArray<FName> GetAllInfantryWeaponIDsOfType(EWeaponType WeaponType) const;
#pragma endregion

    UFUNCTION(BlueprintCallable)
    void PreloadCoreAssets();


    //CPP Only Functions
    const FInfantryWeaponData* GetInfantryWeaponDataRow(FName RowName) const;
    const FWeaponAttachmentData* GetWeaponAttachmentDataRow(FName RowName) const;
    const FVehicleData* GetVehicleDataRow(FName RowName) const;
    const FVehicleWeaponData* GetVehicleWeaponDataRow(FName RowName) const;
    const FVehicleAttachmentData* GetVehicleAttachmentDataRow(FName RowName) const;
    const FOpticData* GetOpticDataRow(FName RowName) const;
    const FProjectileData* GetProjectileDataRow(FName RowName) const;
    const FCamoData* GetCamoDataRow(FName RowName) const;

private:
    //loaded data

    UPROPERTY()
    UDataTable* InfantryWeaponDataTable;

    UPROPERTY()
    UDataTable* WeaponAttachmentDataTable;

    UPROPERTY()
    UDataTable* VehicleDataTable;

    UPROPERTY()
    UDataTable* VehicleWeaponDataTable;

    UPROPERTY()
    UDataTable* ProjectileDataTable;

    UPROPERTY()
    UDataTable* WeaponDataTable;

    UPROPERTY()
    UDataTable* VehicleAttachmentDataTable;

    UPROPERTY()
    UDataTable* OpticDataTable;

    UPROPERTY()
    UDataTable* CamoDataTable;

    UPROPERTY()
    UDA_CoreDefaults* CoreDefaultsDefinitionsDataAsset;

    UPROPERTY()
    UDA_CharacterDefaults* CharacterDefaultsDataAsset;

    UPROPERTY()
    UDA_SoldierClassDefaults* SoldierClassDefaultsDataAsset;

    UPROPERTY()
    UDA_WeaponDefaults* WeaponDefaultsDataAsset;

    UPROPERTY()
    UDA_VehicleDefaults* VehicleDefaultsDataAsset;

    UPROPERTY()
    UDA_CustomizationDefaults* CustomizationDefaultsDataAsset;


    bool bDataReady = false;

};