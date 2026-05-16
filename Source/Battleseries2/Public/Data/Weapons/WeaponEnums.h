#pragma once

#include "CoreMinimal.h"
#include "WeaponEnums.generated.h"

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	AssaultRifle	UMETA(DisplayName = "Assault Rifle"),
	Carbine			UMETA(DisplayName = "Carbine"),
	SMG				UMETA(DisplayName = "Submachine Gun"),
	LMG				UMETA(DisplayName = "Light Machine Gun"),
	Shotgun			UMETA(DisplayName = "Shotgun"),
	DMR				UMETA(DisplayName = "Designated Marksman Rifle"),
	SniperRifle		UMETA(DisplayName = "Sniper Rifle"),
	Pistol			UMETA(DisplayName = "Pistol")
};

UENUM(BlueprintType)
enum class EWeaponFireType : uint8
{
	SimProjectile		    UMETA(DisplayName = "Simulated Projectile"),
	ActorProjectile			UMETA(DisplayName = "Physical/Actor Projectile"),
	VFX						UMETA(DisplayName = "VFX"),
	Hitscan					UMETA(DisplayName = "Hitscan")		//soflam, lazer guided, etc
};

UENUM(BlueprintType)
enum class EFireMode : uint8
{
	Auto		UMETA(DisplayName = "Auto"),
	Burst		UMETA(DisplayName = "Burst"),
	Single		UMETA(DisplayName = "Single/Semi-Auto")
};

#pragma region WeaponAttachments

UENUM(BlueprintType)
enum class EWeaponAttachmentType : uint8
{
	Sight			UMETA(DisplayName = "Sight/Optic/Scope"),
	LaserLight		UMETA(DisplayName = "Laser/Light"),
	Handguard		UMETA(DisplayName = "Handguard"),
	Foregrip		UMETA(DisplayName = "Foregrip"),
	PistolGrip		UMETA(DisplayName = "Pistol Grip"),
	Underbarrel		UMETA(DisplayName = "Underbarrel"),
	Muzzle			UMETA(DisplayName = "Muzzle/Suppressor"),
	Stock			UMETA(DisplayName = "Stock"),
	RailCover		UMETA(DisplayName = "RailCover"),
	Mag				UMETA(DisplayName = "Magazine")
};

UENUM(BlueprintType)
enum class ESightSlot : uint8
{
	FrontSight		UMETA(DisplayName = "Front Sight"),
	RearSight		UMETA(DisplayName = "Rear Sight"),
	Scope			UMETA(DisplayName = "Optic/Scope"),
	Canted			UMETA(DisplayName = "Canted/Flip/Hybrid")
};

#pragma endregion

UENUM(BlueprintType)
enum class ETuningCapability : uint8
{
	NoTuning	UMETA(DisplayName = "No tuning"),
	StatOnly	UMETA(DisplayName = "Stat Only/No Visual"),
	Visual		UMETA(DisplayName = "Visual & Stat Affected")
};

UENUM(BlueprintType)		//The different behaviors of a weapon that dictates under what parameters can it home, lock, or guide to someting
enum class EHomingCapability : uint8
{
	NoHoming			UMETA(DisplayName = "No Homing", ToolTip = "Cannot do any form of Homing Capability"),		
	WireGuided1			UMETA(DisplayName = "Wire Guided (no lock on)", ToolTip = "A wire guided missile that CANNOT lock on to targets"),	
	WireGuided2			UMETA(DisplayName = "Wire Guided (w/lock on)", ToolTip = "A wire guided missile that CAN lock on to targets, TOW Missile, etc"),
	GPSGuidance			UMETA(DisplayName = "GPS Guidance", ToolTip = "Guides to a marked location that does not change (JDAM, etc)"),
	CanLockOn			UMETA(DisplayName = "Lock On", ToolTip = "Passive lock on, does not require a lock to fire"),		
	RequireLockOn		UMETA(DisplayName = "Require Lock On", ToolTip = "Requires lock on to fire")
};

UENUM(BlueprintType)		//what the weapon can target/lock on to
enum class ETargetingCategory : uint8
{
	GroundVehicle		UMETA(DisplayName = "Ground Vehicle"),
	Aircraft			UMETA(DisplayName = "Aircraft"),
	LazedTarget			UMETA(DisplayName = "Lazed Target"),
	MAX					UMETA(Hidden)
};


/**
* STATES
**/

UENUM(BlueprintType)
enum class EAmmoDepletionMethod : uint8
{
	Default		UMETA(DisplayName = "Default (finite ammo)"),
	Heat		UMETA(DisplayName = "Heat/Cooldown based"),
	None		UMETA(DisplayName = "None (infinite ammo)")
};

UENUM(BlueprintType)					//the different states of a lockon weapon
enum class ELockOnState : uint8									
{
	NotLockingOn		UMETA(DisplayName = "Not Locking On"),			//is not locked on and is not locking on to anything. not doing anything
	IsLockingOn			UMETA(DisplayName = "Is Locking On"),			//is currently locking on to something but hasn't acquired it yet
	IsLosingLock		UMETA(DisplayName = "Is Losing Lock"),			//is currently losing the lock on an acquired target
	IsLockedOn			UMETA(DisplayName = "Is Locked On")				//is locked on, has acquired a target
};

UENUM(BlueprintType)
enum class EFireMethod : uint8
{
	Default			UMETA(DisplayName = "Default"),
	Sequential		UMETA(DisplayName = "Sequential/Flip Flop"),
	//sequential backwards?
	Dual			UMETA(DisplayName = "Dual"),
};

UENUM(BlueprintType)
enum class EMuzzleType : uint8						//defines muzzle socket to use (should be used in muzzle socket string name)
{
	Gun				UMETA(DisplayName = "Gun"),
	Cannon			UMETA(DisplayName = "Cannon"),
	Launcher		UMETA(DisplayName = "Launcher")
	//pod?
	//mounted?		//replace mounted projectile with this?
};


