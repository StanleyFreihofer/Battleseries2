#pragma once

#include "CoreMinimal.h"
#include "CoreEnums.generated.h"

//separate into loadout enums?????????????????????????????????

//alot of this is used to route through data manager (gives it the data needed to look up the correct dt)


UENUM(BlueprintType)
enum class ECoreType : uint8
{
	Character		UMETA(DisplayName = "Character"),
	Class			UMETA(DisplayName = "Class/Loadout/Kit"),
	Weapon			UMETA(DisplayName = "Weapon"),
	Vehicle			UMETA(DisplayName = "Vehicle")
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
