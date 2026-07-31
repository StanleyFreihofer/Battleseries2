#pragma once

#include "CoreMinimal.h"
#include "VehicleEnums.generated.h"

UENUM(BlueprintType)
enum class E_MovementType : uint8
{
	GroundVehicle	UMETA(DisplayName = "Ground Vehicle"),
	Helicopter		UMETA(DisplayName = "Helicopter"),
	Jet				UMETA(DisplayName = "Jet"),
	Boat			UMETA(DisplayName = "Boat"),
	VTOL			UMETA(DisplayName = "VTOL"),
	Stationary		UMETA(DisplayName = "Stationary", ToolTip = "CIWS, TOW launcher, artillery guns, etc")	
};

UENUM(BlueprintType)
enum class EVehicleType : uint8
{
	Tank					UMETA(DisplayName = "Tank"),
	TankDestroyer			UMETA(DisplayName = "Tank Destroyer"),
	IFV						UMETA(DisplayName = "Infantry Fighting Vehicle"),
	MobileAA				UMETA(DisplayName = "Mobile AA"),
	AttackHeli				UMETA(DisplayName = "Attack Helicopter"),
	ScoutHeli				UMETA(DisplayName = "Scout Helicopter"),
	FighterJet				UMETA(DisplayName = "Fighter Jet"),
	AttackJet				UMETA(DisplayName = "Attack Jet"),
	CASJet					UMETA(DisplayName = "CAS Jet", ToolTip = "A10 Warthog, SU25 Frogfoot"),
	LTV						UMETA(DisplayName = "LTV/Jeep", ToolTip = "Growler, Dune Buggy, VDV Buggy"),
	APC						UMETA(DisplayName = "APC", ToolTip = "Humvee, Vodnik, MRAP"),
	ATV						UMETA(DisplayName = "ATV"),
	TransportHeli			UMETA(DisplayName = "Transport Helicopter"),
	RHIB					UMETA(DisplayName = "RHIB"),
	MobileArtillery			UMETA(DisplayName = "Mobile Artillery"),
	Other					UMETA(DisplayName = "Other", ToolTip = "Other miscellanious things that just happen to be vehicles (turrets, gadgets such as EOD Bots and Drones)"),
	MAX						UMETA(Hidden)

	//Attack Boat
	//Multirole Jet
	//Bomber
	//gunship
	//VTOL
	//stationary turret?
};
ENUM_RANGE_BY_COUNT(EVehicleType, EVehicleType::MAX);

UENUM(BlueprintType)
enum class EFlightModelType : uint8
{
	Arcade    UMETA(DisplayName = "Arcade"),
	Chaos      UMETA(DisplayName = "Chaos")
};

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
	Remote			UMETA(DisplayName = "Remote")
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

UENUM(BlueprintType)
enum class EWindowedAimAnchor : uint8
{
	FreeAim				UMETA(DisplayName = "Free Aim", ToolTip = "Follows head direction (head rotation based)"),
	FixedHead			UMETA(DisplayName = "Fixed Head", ToolTip = "Aim direction fixed to socket on player character's head"),
	FixedPoint			UMETA(DisplayName = "Fixed POint", ToolTip = "Aim direction fixed to socket on vehicle"),
	Turret				UMETA(DisplayName = "Gimballed", ToolTip = "Follows turret (turret rotation based)"),
	Hull				UMETA(DisplayName = "Fixed", ToolTip = "Aim direction fixed to Hull (direction vehicle is facing)")
};

UENUM(BlueprintType)
enum class EAlternateViewType : uint8
{
	ThirdPerson			UMETA(DisplayName = "Third Person", ToolTip = "Switch to conventional 3rd person view"),
	FirstPerson			UMETA(DisplayName = "First Person", ToolTip = "Switch to head/conventional view if in an alternate view. If already in first person view, this will do nothing"),
	None				UMETA(DisplayName = "None", ToolTip = "no alternate view available")
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


