// DataManagerSubsystem.cpp
#include "Utilities/DataManagerSubsystem.h"
#include "Engine/DataTable.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "Data/Core/CoreTypes.h"
#include "Data/Vehicles/Data_Seat.h"
#include "Data/Weapons/Data_Weapon.h"
#include "Data/Weapons/Data_InfantryWeapon.h"
#include "Data/Weapons/Data_WeaponAttachments.h"
#include "Data/Vehicles/Data_Vehicle.h"
#include "Data/Weapons/Data_VehicleWeapon.h"
#include "Data/Weapons/Data_Projectile.h"
#include "Data/Data_VehicleAttachments.h"
#include "Data/Data_Optics.h"
#include "Data/Data_Camo.h"
#include "Data/Characters/CharacterDefaults.h"
#include "Data/Weapons/WeaponDefaults.h"
#include "Data/Vehicles/VehicleDefaults.h"
#include "Data/Data_Customization.h"
#include "Utilities/GameInstance_Base.h"

//1 BP parent of this
//variabalize dt's instead of paths

UDataManagerSubsystem::UDataManagerSubsystem()
{

}

void UDataManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    LoadDataTables();
}

bool UDataManagerSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Outer) return false;

    UGameInstance_Base* GI = Cast<UGameInstance_Base>(Outer);
    if (GI && GI->DataManagerSubsystemClass)
    {
        //return true if class being check is 1 assigned to GI
        return GetClass() == GI->DataManagerSubsystemClass;
    }

    return false;
}

void UDataManagerSubsystem::LoadDataTables()
{
    FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
    UE_LOG(LogTemp, Warning, TEXT("DataManagerSubsystem instance: %s"), *GetName());
    UE_LOG(LogTemp, Warning, TEXT("VehicleDT: %s"), VehicleDataTableAsset.IsValid() ? TEXT("Valid") : TEXT("Null"));
    UE_LOG(LogTemp, Warning, TEXT("ProjectileDT: %s"), ProjectileDataTableAsset.IsValid() ? TEXT("Valid") : TEXT("Null"));

    InfantryWeaponDataTable = InfantryWeaponDataTableAsset.LoadSynchronous();
    WeaponAttachmentDataTable = WeaponAttachmentDataTableAsset.LoadSynchronous();
    VehicleDataTable = VehicleDataTableAsset.LoadSynchronous();
    VehicleWeaponDataTable = VehicleWeaponDataTableAsset.LoadSynchronous();
    ProjectileDataTable = ProjectileDataTableAsset.LoadSynchronous();
    OpticDataTable = OpticDataTableAsset.LoadSynchronous();
    VehicleAttachmentDataTable = VehicleAttachmentDataTableAsset.LoadSynchronous();
    CamoDataTable = CamoDataTableAsset.LoadSynchronous();

    CoreTypeDefinitionsDataAsset = CoreTypeDefinitionsDAAsset.LoadSynchronous();
    CustomizationDefaultsDataAsset = CustomizationDefaultsDAAsset.LoadSynchronous();
    CharacterDefaultsDataAsset = CharacterDefaultsDAAsset.LoadSynchronous();
    SoldierClassDefaultsDataAsset = SoldierClassDefaultsDAAsset.LoadSynchronous();
    WeaponDefaultsDataAsset = WeaponDefaultsDAAsset.LoadSynchronous();
    VehicleDefaultsDataAsset = VehicleDefaultsDAAsset.LoadSynchronous();

    check(VehicleDataTable && VehicleWeaponDataTable && ProjectileDataTable);

    bDataReady = true;
    UE_LOG(LogTemp, Warning, TEXT("[DataManagerSubsystem::LoadDataTables] Data tables are loaded"));
    OnDataReady.Broadcast();
}

FVehicleData UDataManagerSubsystem::GetVehicleDataRowCopy(FName RowName) const
{
    const FVehicleData* VehicleData = VehicleDataTable->FindRow<FVehicleData>(RowName, TEXT("VehicleDataLookup"));
    return *VehicleData;
}

FVehicleWeaponData UDataManagerSubsystem::GetVehicleWeaponDataRowCopy(FName RowName) const
{
    const FVehicleWeaponData* VehicleWeaponData = VehicleWeaponDataTable->FindRow<FVehicleWeaponData>(RowName, TEXT("VehicleWeaponDataLookup"));
    return *VehicleWeaponData;
}

FVehicleAttachmentData UDataManagerSubsystem::GetVehicleAttachmentDataRowCopy(FName RowName) const
{
    const FVehicleAttachmentData* VehicleAttachmentData = VehicleAttachmentDataTable->FindRow<FVehicleAttachmentData>(RowName, TEXT("AttachmentDataLookup"));
    return *VehicleAttachmentData;
}

FProjectileData UDataManagerSubsystem::GetProjectileDataRowCopy(FName RowName) const
{
    const FProjectileData* ProjectileData = ProjectileDataTable->FindRow<FProjectileData>(RowName, TEXT("ProjectileDataLookup"));
    return *ProjectileData;
}





//CPP Functions
const FInfantryWeaponData* UDataManagerSubsystem::GetInfantryWeaponDataRow(FName RowName) const
{
    const FInfantryWeaponData* InfantryWeaponData = InfantryWeaponDataTable->FindRow<FInfantryWeaponData>(RowName, TEXT("InfantryWeaponDataLookup"));
    return InfantryWeaponData;
}

const FWeaponAttachmentData* UDataManagerSubsystem::GetWeaponAttachmentDataRow(FName RowName) const
{
    const FWeaponAttachmentData* WeaponAttachmentData = WeaponAttachmentDataTable->FindRow<FWeaponAttachmentData>(RowName, TEXT("WeaponAttachmentDataLookup"));
    return WeaponAttachmentData;
}

//const because we want the pointer to be read-only (dont wanna modify table data)
const FVehicleData* UDataManagerSubsystem::GetVehicleDataRow(FName RowName) const
{
    const FVehicleData* VehicleDataPtr = VehicleDataTable->FindRow<FVehicleData>(RowName, TEXT("VehicleDataLookup"));
    return VehicleDataPtr;
}

const FVehicleWeaponData* UDataManagerSubsystem::GetVehicleWeaponDataRow(FName RowName) const
{
    const FVehicleWeaponData* VehicleWeaponDataPtr = VehicleWeaponDataTable->FindRow<FVehicleWeaponData>(RowName, TEXT("VehicleWeaponDataLookup"));
    return VehicleWeaponDataPtr;
}

const FVehicleAttachmentData* UDataManagerSubsystem::GetVehicleAttachmentDataRow(FName RowName) const
{
    const FVehicleAttachmentData* AttachmentDataPtr = VehicleAttachmentDataTable->FindRow<FVehicleAttachmentData>(RowName, TEXT("AttachmentDataLookup"));
    return AttachmentDataPtr;
}

const FOpticData* UDataManagerSubsystem::GetOpticDataRow(FName RowName) const
{
    const FOpticData* OpticDataPtr = OpticDataTable->FindRow<FOpticData>(RowName, TEXT("OpticDataLookup"));
    return OpticDataPtr;
}

const FProjectileData* UDataManagerSubsystem::GetProjectileDataRow(FName RowName) const
{
    const FProjectileData* ProjectileDataPtr = ProjectileDataTable->FindRow<FProjectileData>(RowName, TEXT("ProjectileDataLookup"));
    return ProjectileDataPtr;
}

const FCamoData* UDataManagerSubsystem::GetCamoDataRow(FName RowName) const
{
    const FCamoData* CamoDataPtr = CamoDataTable->FindRow<FCamoData>(RowName, TEXT("CamoDataLookup"));
    return CamoDataPtr;
}

TArray<FName> UDataManagerSubsystem::GetAllInfantryWeaponIDs() const
{
    UDataTable* InfantryWeaponDT = GetInfantryWeaponDataTable();
    TArray<FName> AllInfantryWeaponRowNames = InfantryWeaponDT->GetRowNames();
    return AllInfantryWeaponRowNames;
}

TArray<FName> UDataManagerSubsystem::GetAllVehicleIDs() const
{
    UDataTable* VehicleDT = GetVehicleDataTable();
    TArray<FName> AllVehicleRowNames = VehicleDT->GetRowNames();
    return AllVehicleRowNames;
}

TArray<FName> UDataManagerSubsystem::GetAllProjectileIDs() const
{
    UDataTable* ProjectileDT = GetProjectileDataTable();
    TArray<FName> AllProjectileRowNames = ProjectileDT->GetRowNames();
    return AllProjectileRowNames;
}

FName UDataManagerSubsystem::GetFirstVehicleIDOfType(EVehicleType VehicleType) const
{
    TArray<FName> VehicleIDs = GetAllVehicleIDs();
    FName VehicleIDofType;
    for (FName VehicleID : VehicleIDs)
    {
        const FVehicleData* VehicleData = GetVehicleDataRow(VehicleID);
        if (VehicleData->Vehicle_Type == VehicleType)
        {
            VehicleIDofType = VehicleID;
            break;
        }
    }
    return VehicleIDofType;
}

FText UDataManagerSubsystem::GetWeaponSlotName(int32 WeaponSlotIndex)
{
    FText WeaponSlotName = WeaponDefaultsDataAsset->WeaponDefaults.WeaponSlotNames[WeaponSlotIndex];
    return WeaponSlotName;
}

TArray<FName> UDataManagerSubsystem::GetAllInfantryWeaponIDsOfType(EWeaponType WeaponType) const
{
    TArray<FName> AllInfantryWeaponIDsOfType;
    for (FName WeaponID : GetAllInfantryWeaponIDs())
    {
        const FInfantryWeaponData* InfantryWeaponData = GetInfantryWeaponDataRow(WeaponID);
        if (InfantryWeaponData->WeaponClassificationData.WeaponType == WeaponType)
        {
            AllInfantryWeaponIDsOfType.Add(WeaponID);
        }
    }
    return AllInfantryWeaponIDsOfType;
}

void UDataManagerSubsystem::PreloadCoreAssets()
{
    //FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();
    //TArray<FSoftObjectPath> AssetsToLoad;
    //example: add vehicle meshes, etc

    //StreamableManager.RequestAsyncLoad(AssetsToLoad, FStreamableDelegate::CreateLambda([]()
    //{
      //      UE_LOG(LogTemp, Log, TEXT("Core assets preloaded successfully."));
    //}));
}
