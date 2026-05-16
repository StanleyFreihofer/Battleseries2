#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Utilities/ProjectilePoolSubsystem.h"
#include "Utilities/DataManagerSubsystem.h"
#include "Data/Runtime/WeaponTypes.h"
#include "Data/Core/CoreTypes.h"
#include "Data/Core/CoreEnums.h"
#include "WeaponLogicComponent.generated.h"

class USkeletalMeshComponent;
class UStaticMeshComponent;
class AProjectile_Base;
struct FPlayerLoadoutConfig_Class;


//the base component class for handling runtime weapon data and logic
//this class is used by on-foot weapons/characters

USTRUCT(BlueprintType)
struct FWeaponAttachmentState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPlayerLoadoutConfig_WeaponAttachment BaseAttachmentState = FPlayerLoadoutConfig_WeaponAttachment();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TWeakObjectPtr<UStaticMeshComponent> SpawnedAttachment = nullptr; // The actual mesh on the gun
};

USTRUCT(BlueprintType)
struct FInfantryWeaponState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EAttachmentSlot, FWeaponAttachmentState> WeaponAttachmentStates;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TWeakObjectPtr<USkeletalMeshComponent> WeaponMesh = nullptr;
};

USTRUCT(BlueprintType)
struct FInfantryWeaponSystem
{
	//each array item corresponds to weapon index
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FInfantryWeaponState> WeaponState_FP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FInfantryWeaponState> WeaponState_TP;
};

USTRUCT(BlueprintType)
struct FOnFootWeaponSystem_Runtime
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FBaseWeaponSystem_Runtime BaseWeaponSystem = FBaseWeaponSystem_Runtime();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FInfantryWeaponSystem InfantryWeaponSystem = FInfantryWeaponSystem();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool isAiming = false;

};

UCLASS(Blueprintable, BlueprintType)
class BATTLESERIES2_API UWeaponLogicComponent : public UActorComponent
{
	GENERATED_BODY()

	public:
		UWeaponLogicComponent();
		virtual void BeginPlay() override;
		virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Runtime")
		FOnFootWeaponSystem_Runtime WeaponSystem;
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Runtime")
		FTimerHandle TimerHandle_AutoFire;

		UFUNCTION(BlueprintCallable)
		void Init_WeaponLoadout(FPlayerLoadoutConfig_Class ClassLoadout, TArray<FPlayerLoadoutConfig_Weapon> WeaponLoadouts);
		UFUNCTION(BlueprintCallable)
		void ApplyAttachments(const FPlayerLoadoutConfig_Weapon& AttachmentsToApply, int32 WeaponIndex);
		UFUNCTION(BlueprintCallable)
		virtual bool Rangefinder(const FTransform& StartTransform, FHitResult& OutHit);
		UFUNCTION(BlueprintCallable)			//calculates projectile velocity direction based on rangefinder (projectile should move toward cos, new SetProjectileSpawnTransform())
		virtual void UpdateProjectileAimDirection();		
		UFUNCTION(BlueprintCallable) //precursor to FireWeapon (checks to see if the weapon can be fired and determines nature of fire weapon)
		virtual void StartFire();
		UFUNCTION(BlueprintCallable)
		virtual void CeaseFire();
		UFUNCTION(BlueprintCallable)
		void DryFire();
		UFUNCTION(BlueprintCallable)
		void FireWeapon();

		UFUNCTION(BlueprintCallable, BlueprintPure)
		FName GetSocketNameForSlot(EAttachmentSlot Slot);
		UFUNCTION()
		UDataManagerSubsystem* GetDataManager();

	protected:
		TArray<FBaseWeaponData*> StaticWeaponDataCache;

		FWeapon_Runtime* GetCurrentWeaponRuntime();
		const FBaseWeaponData* GetCurrentWeaponStaticData() const;

	private:
		UProjectilePoolSubsystem* ProjectileManager;
};
