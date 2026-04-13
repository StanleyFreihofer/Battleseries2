#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Utilities/ProjectilePoolSubsystem.h"
#include "Utilities/DataManagerSubsystem.h"
#include "Data/Runtime/WeaponTypes.h"
#include "WeaponLogicComponent.generated.h"

class USkeletalMeshComponent;
class AProjectile_Base;


//the base component class for handling runtime weapon data and logic
//this class is used by on-foot weapons/characters

USTRUCT(BlueprintType)
struct FAttachmentState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName AttachmentID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentRailOffset = 0.0f;		//tuning value

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UStaticMeshComponent> SpawnedComponent = nullptr; // The actual mesh on the gun
};

USTRUCT(BlueprintType)
struct FOnFootWeaponSystem_Runtime
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FBaseWeaponSystem_Runtime BaseWeaponSystem = FBaseWeaponSystem_Runtime();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FAttachmentState> AttachmentState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<USkeletalMeshComponent*> WeaponMeshes;			//the actual weapon mesh (gun model for example)

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
