#pragma once

#include "CoreMinimal.h"
#include "Data/Items/Weapons/WeaponEnums.h"
#include "Data/Items/Weapons/Data_Weapon.h"
#include "WeaponTypes.generated.h"
class AProjectile_Base;
struct FBaseWeaponData;

//shared base layer of weapon runtime data/state

USTRUCT(BlueprintType)
struct FLockOnState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ELockOnState CurrentLockStatus = ELockOnState::NotLockingOn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TWeakObjectPtr<USceneComponent> AcquiredTargetComp = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)		//for laser-guided targets (JDAM's)
	FVector DesignatedPoint = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTimerHandle LockOnTimer = FTimerHandle();
};

USTRUCT(BlueprintType)		
struct FWeaponState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool canFire = true;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool isFiring = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool isReloading = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool isWarmingUp = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool isEquipped = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	EFireMode CurrentFireMode = EFireMode::Auto;		//defined in weapon config data
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 CurrentAmmoinMag = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 CurrentReserveAmmo = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FTimerHandle TimerHandle_AutoFire = FTimerHandle();
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FTimerHandle TimerHandle_Reload = FTimerHandle();

	UPROPERTY(VisibleAnywhere)
	TArray<TWeakObjectPtr<AProjectile_Base>> InFlightProjectiles;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FLockOnState LockOnState = FLockOnState();
};

USTRUCT(BlueprintType)
struct FWeapon_Runtime
{
	//DEPRECIATE/MOVE WEAPONID INTO FWEAPONSTATE
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FName WeaponID = NAME_None;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FWeaponState WeaponState = FWeaponState();
};

USTRUCT(BlueprintType)
struct FWeaponRaycastData_Runtime
{
	GENERATED_BODY()

	FWeaponRaycastData_Runtime()
	{
		// Ensure there is always at least one entry to prevent index out of bounds
		MuzzleAimDirections.Add(FVector::ForwardVector);
	}

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FHitResult RangefinderData = FHitResult();

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FVector> MuzzleAimDirections;
};


USTRUCT(BlueprintType)
struct FEquippedWeaponState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 CurrentWeaponIndex = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FWeaponRaycastData_Runtime RaycastData = FWeaponRaycastData_Runtime();
};

USTRUCT(BlueprintType)
struct FBaseWeaponSystem_Runtime
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray <FWeapon_Runtime> Weapons;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FEquippedWeaponState EquippedWeaponState = FEquippedWeaponState();
};

USTRUCT(BlueprintType)
struct FWeaponStats_Runtime
{
	//used to cache stats of weapon AFTER ATTACHMENTS/UPGRADES HAVE BEEN APPLIED 
	GENERATED_BODY()

	// --- Aim / Handling Performance (Flattened from FInfantryWeaponAimData) ---
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon | Runtime Stats")
	float AimInSpeed = 0.2f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon | Runtime Stats")
	float AimOutSpeed = 0.2f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon | Runtime Stats")
	float SightDistance = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon | Runtime Stats")
	FWeaponFireModeData FireModeData = FWeaponFireModeData();

	// --- Fire Performance (Flattened from FWeaponFirePerformanceData & FWeaponDamageData) ---
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon | Runtime Stats")
	float RateOfFire = 600.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon | Runtime Stats")
	float BaseDamage = 25.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon | Runtime Stats")
	float MuzzleVelocity = 900.0f;

	// --- Magazine Performance (Flattened from FAmmoData) ---
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon | Runtime Stats")
	int32 MagSize = 30;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon | Runtime Stats")
	int32 MaxReserveAmmo = 30;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon | Runtime Stats")
	float ReloadSpeed = 2.5f;
};