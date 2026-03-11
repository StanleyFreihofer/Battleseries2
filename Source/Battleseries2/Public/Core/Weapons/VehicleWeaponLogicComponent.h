#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NiagaraSystem.h"
#include "Utilities/ProjectilePoolSubsystem.h"
#include "Utilities/I_VehicleDataAccessor.h"
#include "Data/Runtime/WeaponTypes.h"
#include "Data/Vehicles/Data_Seat.h"
#include "Data/Runtime/CoreTypes.h"
#include "VehicleWeaponLogicComponent.generated.h"

class USpringArmComponent;
class UCameraComponent;
class USkeletalMeshComponent;
class ACharacter_Base;
class AProjectile_Base;
class UWeaponLogicComponent;
class UDataManagerSubsystem;
class UHUDSubsystem;
struct FWeapon_Runtime;							//WeaponTypes.h
struct FEquippedWeaponState;					//WeaponTypes.h
struct FProjectileData;
struct FWeaponHomingData;

//class for handling all of the weapons equipped on a vehicle
//much logic and some data is used from base weapon logic comp
//REMEMBER THIS SHOULD NOT HAVE A REFERENCE TO VEHICLE

DECLARE_DELEGATE(FOnTriggerReleased);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnVehicleWeaponSystemCleared);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWindowedRangefinder);

USTRUCT(BlueprintType)									//contextualizes base weapon state data w/vehicle weapon specific state
struct FVehicleWeaponState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)			//cached for quick access, contains ID, base State
	FWeapon_Runtime BaseWeaponRuntimeData = FWeapon_Runtime();
	UPROPERTY(BlueprintReadWrite, EditAnywhere)			//more than 1 muzzle (think of M142, any multi-barrel weapon)
	TArray<int32> CurrentMuzzleIndexes;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FName> MuzzleSockets;
	UPROPERTY(EditAnywhere)
	TArray<TWeakObjectPtr<UNiagaraComponent>> MuzzleVFXPool;
	UPROPERTY(EditAnywhere)
	TArray<TWeakObjectPtr<AProjectile_Base>> CurrentMountedProjectiles;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USpringArmComponent* WeaponTurretSpringArm = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ACameraActor* WeaponTurretCamera = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FDecorative_Runtime> VehicleWeaponDecoratives;
};

USTRUCT(BlueprintType)
struct FVehicleWeapon_Runtime						//defines an entire weapon on the vehicle
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVehicleWeaponState VehicleWeaponState = FVehicleWeaponState();

	UPROPERTY(BlueprintReadWrite, EditAnywhere)			//cached for quick access, contains data that defines weapon context on vehicle
	FVehicleWeaponInstanceData VehicleWeaponInstanceData = FVehicleWeaponInstanceData();
};

USTRUCT(BlueprintType)
struct FTurretState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentTurretRotation = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentTurretPitch = 0.0f;
};

USTRUCT(BlueprintType)
struct FVehicleWeaponSystemState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FEquippedWeaponState EquippedWeaponState = FEquippedWeaponState();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)		//weapon mesh/separate turret
	TWeakObjectPtr<USkeletalMeshComponent> WeaponSystemMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TWeakObjectPtr<UAudioComponent> WeaponAudioComponent = nullptr; // Persistent audio slot per seat

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TWeakObjectPtr<UStaticMeshComponent> ReticleQuad = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName WSAttachmentID = NAME_None;
};

USTRUCT(BlueprintType)
struct FVehicleWeaponSystem_Runtime
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVehicleWeapon_Runtime> Weapons;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVehicleWeaponSystemState VehicleWeaponSystemState = FVehicleWeaponSystemState();
};

UCLASS(Blueprintable, BlueprintType)
class BATTLESERIES2_API UVehicleWeaponLogicComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVehicleWeaponLogicComponent();
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//key = seatindex
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<int32, FVehicleWeaponSystem_Runtime> VehicleWeaponSystem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)		//either the rotation of the bone on the vehicle or the separate mesh here
	TArray<FTurretState> TurretStates;

	UPROPERTY(BlueprintAssignable, Category = "Event Dispatchers")
	FOnVehicleWeaponSystemCleared OnVWSCleared;

	UPROPERTY(BlueprintAssignable, Category = "Event Dispatchers")
	FWindowedRangefinder WindowedRangefinder;


	UFUNCTION(BlueprintCallable)
	void Init_VehicleWeaponSystem(TMap<int32, FSavedSeatLoadout> SeatLoadouts);

	UFUNCTION(BlueprintCallable)
	void Init_Turrets(int32 NumOfTurrets);

	UFUNCTION(BlueprintCallable)
	void ApplyWeaponInstanceDataAtIndexToSeat(int32 SeatIndex, int32 WeaponIndex, FName WeaponID);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float CalculateTurret(float InputValue, float TurretSpeed, float CurrentTurretValue);

	UFUNCTION(BlueprintCallable)
	void ControlTurret(FVector2D InputValue, int32 SeatIndex);

	UFUNCTION(BlueprintCallable)
	void UpdateTurretMesh(int32 SeatIndex, float TurretRotation, float TurretPitch);

	UFUNCTION(BlueprintCallable)
	void HandleApplyWeaponsToSeat(int32 SeatIndex);

	UFUNCTION(BlueprintCallable)
	void ApplyWeaponsToSeat(int32 SeatIndex, TArray<FName> WeaponIDs);

	UFUNCTION(BlueprintCallable)
	void ApplyWeaponAtIndexToSeat(int32 SeatIndex, int32 WeaponIndex, FName WeaponID);

	UFUNCTION(BlueprintCallable)
	void MountProjectiles(int32 SeatIndex, int32 WeaponIndex);
	UFUNCTION(BlueprintCallable)
	void ApplyWeaponDecoratives(const TArray<FDecorative>& WeaponDecoratives, FVehicleWeapon_Runtime& RuntimeWeaponData);

	UFUNCTION(BlueprintCallable)
	const FVehicleWeaponInstanceData& GetWeaponInstanceDataAtSlotInSeat(int32 SeatIndex, int32 WeaponIndex, FName WeaponID);

	UFUNCTION()
	TWeakObjectPtr<AActor> GetCurrentViewTargetAtSeatIndex(int32 SeatIndex);

	UFUNCTION(BlueprintCallable)
	USkeletalMeshComponent* ApplyWeaponMeshToVehicle(USkeletalMesh* Mesh, int32 SeatIndex, FTransform MeshTransform);

	UFUNCTION(BlueprintCallable)
	void ConfigureWeaponCam(int32 SeatIndex, int32 WeaponIndex, FVehicleWeaponSystem_Runtime& WeaponSystem);

	UFUNCTION(BlueprintCallable)
	void ClearWeaponSlotFromSeat(int32 SeatIndex, int32 WeaponIndex);

	UFUNCTION(BlueprintCallable)
	void ClearWeaponSystemFromSeat(int32 SeatIndex, bool RemoveFromMap);
	UFUNCTION(BlueprintCallable)
	void ClearEntireWeaponSystemFromVehicle();

	UFUNCTION(BlueprintCallable)
	void HandleSeatRangefinders();
	UFUNCTION(BlueprintCallable)
	void UpdateSeatRangefinder(int32 SeatIndex, FTransform TraceTransform, TArray<AActor*> ActorsToIgnore);
	UFUNCTION(BlueprintCallable)
	void StartLockingOn(int32& SeatIndex, FVehicleWeapon_Runtime& CurrentWeapon, const FWeaponHomingData& HomingData, const FHitResult& HitResult);
	UFUNCTION(BlueprintCallable)
	void LockOn(int32 SeatIndex, const FWeaponHomingData HomingData, const FHitResult HitResult);
	UFUNCTION(BlueprintCallable)
	void StartCancelLockOn(int32& SeatIndex, FLockOnState& LockOnState);
	UFUNCTION(BlueprintCallable)
	void CancelLockOn(int32 SeatIndex, int32 WeaponIndex);

	UFUNCTION(BlueprintCallable)
	void UpdateLockOnIndicator(bool UpdateHUD, FHitResult& HitResult, FLockOnState& LockOnState);

	UFUNCTION(BlueprintCallable)
	FName BuildMuzzleName(FName SeatName, int32 WeaponIndex, EMuzzleType MuzzleType, int32 MuzzleIndex);
	UFUNCTION()
	void SetupMuzzleSockets(TWeakObjectPtr<USkeletalMeshComponent> Mesh, int32 SeatIndex, int32 WeaponIndex, FName SeatName);
	UFUNCTION()
	TArray<FName> GetMuzzleSocketsInOrder(TWeakObjectPtr<USkeletalMeshComponent> Mesh, FName SeatName, int32 WeaponIndex, EMuzzleType MuzzleType);
	UFUNCTION()
	void CalculateAimDirection(TWeakObjectPtr<USkeletalMeshComponent> Mesh, FHitResult HitResult, int32 WeaponIndex, int32 SeatIndex);
	UFUNCTION(BlueprintCallable)
	void HandleStartFire(int32 SeatIndex);
	UFUNCTION(BlueprintCallable)
	void StartFire(int32 SeatIndex);
	UFUNCTION(BlueprintCallable)
	void StartWeaponFireAudio(int32 SeatIndex);
	UFUNCTION(BlueprintCallable)
	void FireVehicleWeapon(int32 SeatIndex);
	UFUNCTION(BlueprintCallable)
	void HandleShootSimProjectile(FVehicleWeaponState& VehicleWeaponState, const FBaseWeaponData& StaticWeaponData, FVehicleWeaponSystem_Runtime& SeatWeaponSystem, const FProjectileData& ProjectileData);
	UFUNCTION(BlueprintCallable)
	void HandleAmmoDepletion(const FBaseWeaponData& StaticWeaponData, FWeapon_Runtime& CurrentWeapon, int32 SeatIndex);
	UFUNCTION(BlueprintCallable)
	void HandleStartAutoload(int32 SeatIndex);
	UFUNCTION(BlueprintCallable)
	void StartAutoload(const FBaseWeaponData& StaticWeaponData, int32 SeatIndex, int32 WeaponIndex);
	UFUNCTION(BlueprintCallable)
	void AutoloadNewMag(int32 SeatIndex, int32 WeaponIndex, int32 MagSize);
	UFUNCTION(BlueprintCallable)
	void ApplyWeaponRecoilJostle(int32 SeatIndex, int32 WeaponIndex);
	UFUNCTION(BlueprintCallable)
	void StopFire(int32 SeatIndex);
	UFUNCTION(BlueprintCallable)
	void StopWeaponSlotFire(int32 SeatIndex, int32 WeaponIndex);
	UFUNCTION(BlueprintCallable)
	void SwitchWeapon(int32 SeatIndex);
	UFUNCTION(BlueprintCallable)
	void SelectWeapon(int32 SeatIndex, int32 WeaponIndex);
	UFUNCTION(BlueprintCallable)
	void UnequipWeapon(int32& SeatIndex, int32& WeaponIndex, bool& bWasFiring);
	UFUNCTION(BlueprintCallable)
	void UpdateWeaponAudioCompData(int32 SeatIndex, int32 WeaponIndex);
	UFUNCTION(BlueprintCallable)
	void UpdateWeaponStatusUI(int32& SeatIndex, bool& canFire);

	//GETTERS
	UFUNCTION(BlueprintCallable, BlueprintPure)
	const FBaseWeaponData& GetBaseWeaponDataInSlot(int32 SeatIndex, int32 WeaponIndex);
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetTurretWorldYaw(int32 TurretIndex);
	UFUNCTION(BlueprintCallable, BlueprintPure)
	int32 GetSeatIndexForTurret(int32 TurretIndex);
	UFUNCTION(BlueprintCallable, BlueprintPure)
	int32& GetCWIForSeat(int32 SeatIndex);
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FVehicleWeapon_Runtime& GetEquippedWeaponInSeat(int32 SeatIndex);
	UFUNCTION()
	TWeakObjectPtr<UHUDSubsystem> GetHUDSystem();
	UFUNCTION()
	TWeakObjectPtr<UAudioComponent> GetWAC(int32& SeatIndex);

protected:
	//(SeatIndex, [WeaponIndex] BaseWeaponData)
	TMap<int32, TArray<const FBaseWeaponData*>> CurrentVehicleBaseWeaponData;		//the static base weapon data of each weapon slot in each seat 
	//current VehicleInstanceWeaponData?


private:
	UDataManagerSubsystem* DataSubsystem;
	UProjectilePoolSubsystem* ProjectileSubsystem;
	IVehicleDataAccessor* OwnerDataAccessor;

	FTimerHandle TimerHandle_AutoFire;			//put into actual architecture/move somewhere else
	FTimerHandle TimerHandle_Reload;			//put into actual architecture/move somewhere else (per weapon, weapons need to track their own progress)
	FTimerHandle TimerHandle_ReserveResupply; //put into actual architecture/move somewhere else
};
