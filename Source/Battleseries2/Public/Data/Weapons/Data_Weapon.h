#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "NiagaraSystem.h"
#include "Data/Weapons/WeaponEnums.h"
#include "Data_Weapon.generated.h"

//generic weapon data (to be expanded upon by vehicle and on-foot weaponry data)


USTRUCT(BlueprintType)
struct FBaseWeaponClassificationData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText WeaponDisplayName = FText();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)		//abbreviated display name (for use in things like in-game huds and stuff)
	FText WeaponDisplayNameAbrev = FText();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText WeaponDescription = FText();
};

USTRUCT(BlueprintType)
struct FWeaponHomingData
{
	GENERATED_BODY()

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "whether or not weapon requires a lockon in order to fire"))
	//bool RequiresLockOn = false;			
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "the parameters that dictate if/HOW a weapon can guide to a target"), Category = "PreFlight Conditions")
	EHomingCapability HomingCapability = EHomingCapability::NoHoming;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "the parameters that dictate what a weapon can target/lock on to"), Category = "PreFlight Conditions")
	TArray<ETargetingCategory> CanTarget;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "the maximum distance a lock on target can be in order to get a lockon"), Category = "PreFlight Conditions")
	float LockOnRange = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "time it takes to lockon/acquire a target"), Category = "PreFlight Conditions")
	float AcquireTime = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "Soft/Hard Lock. Does it require the player to keep crosshairs on target"), Category = "MidFlight Conditions")
	bool ContinuousLockRequired = false;
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

	float GetDamageAtDistance(float Distance) const
	{
		if (DamageDropoffCurve)
		{
			// If the curve exists, it defines the Max/Min behavior entirely
			return DamageDropoffCurve->GetFloatValue(Distance);
		}
		return BaseDamage;
	}
};

USTRUCT(BlueprintType)
struct FWeaponFirePerformanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EWeaponFireType WeaponFireType = EWeaponFireType::SimProjectile;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditConition = "WeaponFireType == EWeaponFireType::ActorProjectile", EditConditionHides = true))
	FName MunitionID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RateOfFire = 0.0f;

	// --- SIMULATED BALLISTICS (Visible only for Sim) ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "WeaponFireType == EWeaponFireType::SimProjectile", EditConditionHides = true))
	float MuzzleVelocity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "WeaponFireType == EWeaponFireType::SimProjectile", EditConditionHides = true))
	float GravityScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FWeaponDamageData WeaponDamageData = FWeaponDamageData();
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
	FBaseWeaponClassificationData WeaponClassification = FBaseWeaponClassificationData();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FWeaponFunctionalityData WeaponFunctionality = FWeaponFunctionalityData();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FWeaponFirePerformanceData WeaponFirePerformance = FWeaponFirePerformanceData();

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
