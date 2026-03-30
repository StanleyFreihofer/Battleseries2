#pragma once

#include "CoreMinimal.h"
#include "Data/Weapons/WeaponEnums.h"
#include "WeaponTypes.generated.h"
class AProjectile_Base;
struct FBaseWeaponData;

USTRUCT(BlueprintType)
struct FLockOnState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ELockOnState CurrentLockStatus = ELockOnState::NotLockingOn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TWeakObjectPtr<AActor> AcquiredTarget = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)		//for laser-guided targets (JDAM's)
	FVector DesignatedPoint = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTimerHandle LockOnTimer;

	//projectile array (current homing projectiles in air?), 
	// map with their own acquire target... no if they have an acquired target, the projectiles will have that cached

	//potential lock on target? A thing acquired that can be locked on but hasn't actually been locked onto yet (should be a local var in whatever function)
};

USTRUCT(BlueprintType)		//Defines the structure of the runtime state
struct FWeaponState
{
	//this is the weapon's state, meaning every property here can change at runtime (unlike instance data which SHOULD NOT change @runtime)
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool canFire = true;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool isFiring = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool isReloading = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool isWarmingUp = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool isEquipped = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	EFireMode CurrentFireMode = EFireMode::Auto;		//defined in weapon config data
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 CurrentAmmoinMag = 0;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 CurrentReserveAmmo = 0;

	//in flight projectiles? (actors mainly?)
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FLockOnState LockOnState = FLockOnState();
};

//1 weapon's runtime data
//gets set from Data_VehicleWeapon @runtime
USTRUCT(BlueprintType)
struct FWeapon_Runtime
{
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