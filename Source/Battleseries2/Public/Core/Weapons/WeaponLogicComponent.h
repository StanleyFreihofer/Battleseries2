#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Utilities/ProjectilePoolSubsystem.h"
#include "Utilities/DataManagerSubsystem.h"
#include "Data/Runtime/WeaponTypes.h"
#include "WeaponLogicComponent.generated.h"

class USkeletalMeshComponent;
class UStaticMeshComponent;
class AProjectile_Base;


//the base component class for handling runtime weapon data and logic
//this class is used by on-foot weapons/characters

USTRUCT(BlueprintType)
struct FWeaponAttachmentState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName AttachmentID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentRailOffset = 0.0f;		//tuning value

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UStaticMeshComponent> SpawnedAttachment = nullptr; // The actual mesh on the gun
};

USTRUCT(BlueprintType)
struct FInfantryWeaponSystem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FWeaponAttachmentState> WeaponAttachmentStates;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<USkeletalMeshComponent*> WeaponMeshes_FP;			//the actual weapon mesh (gun model for example)

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<USkeletalMeshComponent*> WeaponMeshes_TP;			//the actual weapon mesh (gun model for example)
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
		void Init_WeaponLoadout();
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

	protected:
		TArray<FBaseWeaponData*> StaticWeaponDataCache;

		FWeapon_Runtime* GetCurrentWeaponRuntime();
		const FBaseWeaponData* GetCurrentWeaponStaticData() const;

	private:
		UDataManagerSubsystem* DataManager;
		UProjectilePoolSubsystem* ProjectileManager;
};
