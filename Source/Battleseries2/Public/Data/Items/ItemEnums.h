#pragma once

#include "CoreMinimal.h"
#include "ItemEnums.generated.h"

//items & loadout enums

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
enum class ECharacterItemType : uint8				//characterloadoutitemcategory
{
	Weapon				UMETA(DisplayName = "Weapon"),		//fires something
	Gadget				UMETA(DisplayName = "Gadget"),		//placeable gadget
	Grenade				UMETA(DisplayName = "Grenade"),		
	Melee				UMETA(DisplayName = "Melee")
	//specialization/upgrade
	//camo
};

UENUM(BlueprintType)
enum class ELoadoutSlot : uint8
{
	PrimaryWeapon		UMETA(DisplayName = "Primary Weapon"),
	SecondaryWeapon		UMETA(DisplayName = "Secondary Weapon"),
	Gadget1				UMETA(DisplayName = "Signature Class/Default Gadget", ToolTip = "Default Passive class defining gadget, shouldn't be changed (eg. Defib, Repair Tool, Ammo Box)"),
	Gadget2				UMETA(DisplayName = "Optional Gadget 1", ToolTip = "Secondary gadget that can be selected"),
	Gadget3				UMETA(DisplayName = "Optional Gadget 2", ToolTip = "Tertiary gadget that can be selected (only an option if enought inventory/weight slots/room)"),
	Grenade				UMETA(DisplayName = "Grenade"),
	Melee				UMETA(DisplayName = "Melee")
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