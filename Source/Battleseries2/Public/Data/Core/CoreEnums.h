#pragma once

#include "CoreMinimal.h"
#include "CoreEnums.generated.h"

//alot of this is used to route through data manager (gives it the data needed to look up the correct dt)

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
enum class EAttachmentSlot : uint8
{
	FrontSight		UMETA(DisplayName = "Front Sight"),
	RearSight		UMETA(DisplayName = "Rear Sight"),
	Barrel			UMETA(DisplayName = "Barrel"),
	Handguard		UMETA(DisplayName = "Handguard"),
	Scope			UMETA(DisplayName = "Scope"),
	ScopeAccessory	UMETA(DisplayName = "Scope Accessory"),
	Muzzle			UMETA(DisplayName = "Muzzle"),
	Underbarrel		UMETA(DisplayName = "Underbarrel"),		//grip?
	LeftRail		UMETA(DisplayName = "Side Rail Left"),
	RightRail		UMETA(DisplayName = "Side Rail Rght"),
	TopRail			UMETA(DisplayName = "Top Rail Front"),
	PistolGrip		UMETA(DisplayName = "PistolGrip"),
	Stock			UMETA(DisplayName = "Stock"),
	Magazine		UMETA(DisplayName = "Magazine")

	//top rail rear?
	//grip?
	//bottom rail?
	//charm?
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
enum class EInteractType : uint8
{
	Press			UMETA(DisplayName = "Press"),
	Hold			UMETA(DisplayName = "Hold"),
	Passive			UMETA(DisplayName = "Passive", ToolTip = "No input needed to interact")
};

UENUM(BlueprintType)
enum class EActionType : uint8
{
	Pickup			UMETA(DisplayName = "Pickup"),
	Enter			UMETA(DisplayName = "Enter"),
	Open			UMETA(DisplayName = "Open"),
	Custom			UMETA(DisplayName = "Custom")
};

UENUM(BlueprintType)
enum class EFactionType : uint8
{
	US				UMETA(DisplayName = "US"),
	Russia			UMETA(DisplayName = "Russia"),
	China			UMETA(DisplayName = "China")
};

UENUM(BlueprintType)
enum class EClassType : uint8
{
	Medic		UMETA(DisplayName = "Medic"),
	Engineer	UMETA(DisplayName = "Engineer"),
	Support		UMETA(DisplayName = "Support"),
	Recon		UMETA(DisplayName = "Recon"),
	MAX			UMETA(Hidden)
};
ENUM_RANGE_BY_COUNT(EClassType, EClassType::MAX);


/**
* Stat Modifier Enums
**/

UENUM(BlueprintType)
enum class EStatToAffect : uint8		//separate into entity categories (weapons, vehicles, health, etc)?
{
	Health,
	Recoil,
	MuzzleVelocity,
	MagSize,
	MaxReserveAmmo,
	ADS_Speed
};

UENUM(BlueprintType)
enum class EModifierOp : uint8
{
	Add				UMETA(DisplyName = "Add"),
	Subtract		UMETA(DisplayName = "Subtract"),
	Multiply		UMETA(DisplayName = "Multiply"),
	Divide			UMETA(DisplayName = "Divide"),
	Set				UMETA(DisplayName = "Set/Override")
	//inverse (if bool is on, turn off for example)?
};
