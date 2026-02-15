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
	Pistol			UMETA(DisplayName = "Pistol"),
	Vehicle			UMETA(DisplayName = "Vehicle")
	//special
	//launcher
	//stationary
	//melee
};

UENUM(BlueprintType)
enum class EWeaponFireType : uint8
{
	Projectile		    UMETA(DisplayName = "Projectile (Sim or Actor)"),
	VFX					UMETA(DisplayName = "VFX"),
	Hitscan				UMETA(DisplayName = "Hitscan")		//soflam, lazer guided, etc
};

UENUM(BlueprintType)
enum class EFireMode : uint8
{
	Auto		UMETA(DisplayName = "Auto"),
	Burst		UMETA(DisplayName = "Burst"),
	Single		UMETA(DisplayName = "Single")
};

UENUM(BlueprintType)		//The different behaviors of a weapon that dictates under what parameters can it lock on to someting
enum class EHomingCapability : uint8
{
	NoHoming			UMETA(DisplayName = "No Lock On", ToolTip = "Cannot lock on to a target"),		
	WireGuided			UMETA(DisplayName = "Wire Guided", ToolTip = "TOW Missile, etc"),			
	CanLockOn			UMETA(DisplayName = "Lock On", ToolTip = "Can lock on without target being lazed"),		
	RequireLockOn		UMETA(DisplayName = "Require Lock On", ToolTip = "Requires lock on to fire")
	//soft lockon
	//CanHardLockOn		UMETA(DisplayName = "Hard Lock On"),		//Can only lock on if target is lazed (this can be covered by category)
};


UENUM(BlueprintType)		//what the weapon can target/lock on to
enum class ETargetingCategory : uint8
{
	GroundVehicle		UMETA(DisplayName = "Ground Vehicle"),
	Aircraft			UMETA(DisplayName = "Aircraft"),
	AnyVehicle			UMETA(DisplayName = "Any Vehicle"),
	LazedTarget			UMETA(DisplayName = "Lazed Target"),
	Location			UMETA(DisplayName = "Location"),
	MAX					UMETA(Hidden)
};

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
	FlipFlop		UMETA(DisplayName = "Flip Flop"),
	//left right/right left instead?
	Dual			UMETA(DisplayName = "Dual"),
	Sequential		UMETA(DisplayName = "Sequential")
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


