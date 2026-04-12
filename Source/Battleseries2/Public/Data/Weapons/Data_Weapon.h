#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "NiagaraSystem.h"
#include "Data/Weapons/WeaponEnums.h"
#include "Data_Weapon.generated.h"

//generic weapon data (to be expanded upon by vehicle and on-foot weaponry data)


USTRUCT(BlueprintType)
struct FBaseClassificationData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText WeaponDisplayName = FText();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)		//abbreviated display name (for use in things like in-game huds and stuff)
	FText WeaponDisplayNameAbrev = FText();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText WeaponDescription = FText();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EWeaponType WeaponType = EWeaponType::AssaultRifle;
};

USTRUCT(BlueprintType)
struct FWeaponHomingData
{
	GENERATED_BODY()

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "whether or not weapon requires a lockon in order to fire"))
	//bool RequiresLockOn = false;			
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "the parameters that dictate if/HOW a weapon can guide to a target"))
	EHomingCapability HomingCapability = EHomingCapability::NoHoming;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "the parameters that dictate what a weapon can target/lock on to"))
	TArray<ETargetingCategory> CanTarget;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "time it takes to lockon/acquire a target"))
	float AcquireTime = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<USoundWave> LockOnAudio = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<USoundWave> LockedOnAudio = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UUserWidget> IndicatorReticle = nullptr;
};

USTRUCT(BlueprintType)
struct FWeaponFunctionalityData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EFireMode DefaultFireMode = EFireMode::Auto;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool canFullAuto = true;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool canBurstFire = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool canSingleFire = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "canBurstFire", EditConditionHides = true, ToolTip = "the # of rounds in 1 burst"))
	int32 BurstSize = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FWeaponHomingData HomingFunctionality = FWeaponHomingData();
};

//weaponfirehandlingdata (accuracy, recoil, spread, etc)
//weaponhandlingdata (reload time/speed, etc)
USTRUCT(BlueprintType)
struct FWeaponDamageData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float BaseDamage = 0.0f;								//weapons "raw" damage without any falloff

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UCurveFloat* DamageDropoffCurve = nullptr;
};

USTRUCT(BlueprintType)
struct FWeaponPerformanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)		//the initial velocity of whatever projectile when fired
	float MuzzleVelocity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RateOfFire = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FWeaponDamageData WeaponDamageData = FWeaponDamageData();
};

USTRUCT(BlueprintType)
struct FWeaponFireData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EWeaponFireType WeaponFireType = EWeaponFireType::Projectile;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ProjectileID = NAME_None;
};

USTRUCT(BlueprintType)
struct FWeaponAudioData
{
	GENERATED_BODY()

	// --- EXTERNAL / MUZZLE ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Exterior")
	TArray<TSoftObjectPtr<USoundWave>> FireLoop; // The "Bangs" (Array for variance)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Exterior")
	TSoftObjectPtr<USoundWave> FireStop; // Exterior echo/tail

	//what should be vehicle instance data?

	// --- INTERNAL / MECHANICAL (The "Hammer" & "Slide") ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interior")
	TArray<TSoftObjectPtr<USoundWave>> MechanicalImpacts; // The "Hammer" hitting the anvil

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interior")
	TSoftObjectPtr<USoundWave> MechanicalCycle; // The "Slide" of the breech block

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interior")
	TSoftObjectPtr<USoundWave> InteriorTail; // The "Ring" inside the metal cabin
};

USTRUCT(BlueprintType)
struct FWeaponVisualData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "VFX")
	TSoftObjectPtr<UNiagaraSystem> MuzzleFlashFX;

	UPROPERTY(EditAnywhere, Category = "VFX")
	TSoftObjectPtr<UNiagaraSystem> TracerFX;
};

USTRUCT(BlueprintType)
struct FAmmoData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EAmmoDepletionMethod AmmoDepletionMethod = EAmmoDepletionMethod::Default;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "Maximum ammo in one magazine"))
	int32 MagSize = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "Maximum reserve ammo for the weapon"))
	int32 MaxReserveAmmo = 0;					//<-- this ties it to the gun, not the player inventory (if there even is one... BR mode)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "Time it takes to reload"))
	float ReloadSpeed = 0.0f;		//is this gonna work with anim-based reloads?

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool AutoRefillReserve = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "Time it takes between 1 mag size reserve replenish"))
	float RefillReserveSpeed = 0.0f;
};

//culmnination of everything in this data container
//data/properties that ALL weapons get
//NOT THE DATA Table, onfoot weapon and vehicle weapon data containers wil have that
USTRUCT(BlueprintType)
struct FBaseWeaponData 
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FBaseClassificationData WeaponClassification = FBaseClassificationData();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FWeaponFunctionalityData WeaponFunctionality = FWeaponFunctionalityData();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FWeaponFireData WeaponFireData = FWeaponFireData();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FWeaponPerformanceData WeaponPerformance = FWeaponPerformanceData();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FWeaponAudioData WeaponAudio = FWeaponAudioData();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FWeaponVisualData WeaponFX = FWeaponVisualData();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FAmmoData AmmoData = FAmmoData();
};

//special attributes
//lazes/designates
//autofires (no trigger pull needed)
