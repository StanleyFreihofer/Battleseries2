#pragma once

#include "CoreMinimal.h"
#include "Components/SpotLightComponent.h"
#include "Engine/DataAsset.h"
#include "CoreTypes.generated.h"

/**
ALL THE DIFFERENT ITEMS/THINGS THAT MAKE UP AN ENTITY (make up a character class, vehicle, weapon, etc)
alot of this is very useful/used for customization
use to route through data manager (gives it the data needed to look up the correct dt)
**/

UENUM(BlueprintType)
enum class EVehicleItemType : uint8				
{
	Weapon					UMETA(DisplayName = "Weapon"),
	Countermeasure			UMETA(DisplayName = "Countermeasure"),
	Optic					UMETA(DisplayName = "Optic"),
	Upgrade					UMETA(DisplayName = "Upgrade"),
	//Gadget					UMETA(DisplayName = "Gadget")
	//Equipment
	//Camo
};

UENUM(BlueprintType)
enum class ECharacterItemType : uint8
{
	Weapon				UMETA(DisplayName = "Weapon"),
	Gadget				UMETA(DisplayName = "Gadget"),
	Grenade				UMETA(DisplayName = "Grenade")
	//knife
	//specialization/upgrade
	//camo
};

UENUM(BlueprintType)
enum class EWeaponItemType : uint8
{
	Scope			UMETA(DisplayName = "Scope"),
	//laser
	//grip
	//stock
	//muzzle
};

UENUM(BlueprintType)
enum class ECoreType : uint8
{
	Character		UMETA(DisplayName = "Character"),
	Class			UMETA(DisplayName = "Class/Loadout/Kit"),
	Weapon			UMETA(DisplayName = "Weapon"),
	Vehicle			UMETA(DisplayName = "Vehicle")
};

UENUM(BlueprintType)
enum class ECoreItemType : uint8
{
	CharacterWeapon				UMETA(DisplayName = "Character Weapon"),
	CharacterGadget				UMETA(DisplayName = "Character Gadget"),
	//character grenade
	//character knife
	VehicleWeapon				UMETA(DisplayName = "Vehicle Weapon"),
	VehicleOptic				UMETA(DisplayName = "Vehicle Optic"),
	VehicleCountermeasure		UMETA(DisplayName = "Vehicle Countermeasure"),
	VehicleUpgrade				UMETA(DisplayName = "Vehicle Upgrade"),
	Camo						UMETA(DisplayName = "Camo")
};

UENUM(BlueprintType)
enum class EFactionType : uint8
{
	US		UMETA(DisplayName = "US"),
	Russia	UMETA(DisplayName = "Russia"),
	China	UMETA(DisplayName = "China")
};

USTRUCT(BlueprintType)
struct FDecorative_Runtime
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TWeakObjectPtr<UStaticMeshComponent> DecorativeMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TWeakObjectPtr<USpotLightComponent> DecorativeSpotLight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName DecorativeAttachmentID = NAME_None;
};

USTRUCT(BlueprintType)
struct FSavedSeatLoadout
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> Weapons;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Optic;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Upgrade;
};

USTRUCT(BlueprintType)
struct FPlayerLoadoutConfig_Vehicle
{
	GENERATED_BODY()

	//Map Seat Index with array of selected weapons, etc FOR THAT SEAT (Seat Index, WeaponIDs)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<int32, FSavedSeatLoadout> SeatLoadout;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleCamo;
};

USTRUCT(BlueprintType)
struct FCoreTypeEnumDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DisplayName;
};

UCLASS(BlueprintType)
class BATTLESERIES2_API UDA_CoreTypes : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<ECoreType, FCoreTypeEnumDefinition>  CoreTypeDefinitions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<ECoreItemType, FCoreTypeEnumDefinition> CoreItemTypeDefinitions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<ECharacterItemType, FCoreTypeEnumDefinition> CharacterItemTypeDefinitions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<EWeaponItemType, FCoreTypeEnumDefinition> WeaponItemTypeDefinitions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<EVehicleItemType, FCoreTypeEnumDefinition> VehicleItemTypeDefinitions;
};




