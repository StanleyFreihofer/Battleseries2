#pragma once

#include "CoreMinimal.h"
#include "VehicleEnums.generated.h"

UENUM(BlueprintType)
enum class E_MovementType : uint8
{
	GroundVehicle	UMETA(DisplayName = "Ground Vehicle"),
	Helicopter		UMETA(DisplayName = "Helicopter"),
	Jet				UMETA(DisplayName = "Jet"),
	Boat			UMETA(DisplayName = "Boat")
	//stationary?
	//vtol?
};

UENUM(BlueprintType)
enum class E_VehicleType : uint8
{
	Tank					UMETA(DisplayName = "Tank"),
	TankDestroyer			UMETA(DisplayName = "Tank Destroyer"),
	IFV						UMETA(DisplayName = "Infantry Fighting Vehicle"),
	MobileAA				UMETA(DisplayName = "Mobile AA"),
	AttackHeli				UMETA(DisplayName = "Attack Helicopter"),
	ScoutHeli				UMETA(DisplayName = "Scout Helicopter"),
	FighterJet				UMETA(DisplayName = "Fighter Jet"),
	AttackJet				UMETA(DisplayName = "Attack Jet"),
	LTV						UMETA(DisplayName = "LTV/Jeep", ToolTip = "Growler, Dune Buggy, VDV Buggy"),
	APC						UMETA(DisplayName = "APC", ToolTip = "Humvee, Vodnik, MRAP"),
	ATV						UMETA(DisplayName = "ATV"),
	TransportHeli			UMETA(DisplayName = "Transport Helicopter"),
	RHIB					UMETA(DisplayName = "RHIB"),
	MobileArtillery			UMETA(DisplayName = "Mobile Artillery"),
	MAX						UMETA(Hidden)

	//Attack Boat
	//Multirole Jet
	//CAS Jet
	//Bomber
	//VTOL
	//stationary turret?
};
ENUM_RANGE_BY_COUNT(E_VehicleType, E_VehicleType::MAX);

UENUM(BlueprintType)
enum class E_SeatRole : uint8
{
	Driver			UMETA(DisplayName = "Driver"),
	Gunner			UMETA(DisplayName = "Gunner"),
	DriverGunner	UMETA(DisplayName = "Driver Gunner"),
	Passenger		UMETA(DisplayName = "Passenger")
	//passenger that can use onfoot weapons (rider?)
};

UENUM(BlueprintType)					//the default method
enum class E_ViewMethod : uint8
{
	Windowed		UMETA(DisplayName = "Windowed"),
	Remote			UMETA(DisplayName = "Remote"),
	//Separate
	//TBD				UMETA(DisplayName = "TBD")
};


UENUM(BlueprintType)
enum class EVehicleWeaponCamMountMethod : uint8
{
	VehicleMesh			UMETA(DisplayName = "Mount to vehicle mesh"),
	WeaponMesh			UMETA(DisplayName = "Mount to weapon system mesh/turret"),
	MountedProjectile	UMETA(DisplayName = "Mount to mounted projectile")
};

UENUM(BlueprintType)
enum class EVehicleWeaponCamActivationMethod : uint8
{
	Equip				UMETA(DisplayName = "Activate cam on equip/switch to (default)"),
	Aim					UMETA(DisplayName = "Activate on aim")
};

UENUM(BlueprintType)						//denotes what specific mesh on a vehicle a camo MI/texture can be applied to
enum class EVehicleCamoMeshOptions : uint8
{
	MainBody,
	WeaponTurret,
	Decoratives
	//Weapon Decorative
	//Upgrade Decorative
	//Armor Decroative
};

UENUM(BlueprintType)
enum class EVehicleHUDType : uint8
{
	Rangefinder,
	Compass,
	Reticle,
	Optic,
	Turret,
	Weapon,
	Countermeasure,
	Movement
};

UENUM(BlueprintType)
enum class ETargetedStatus : uint8
{
	NotLockedOn,
	GettingLockedOn,
	MissileIncoming
};