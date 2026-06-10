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
	FBaseWeaponSystem_Runtime BaseWeaponSystem = FBaseWeaponSystem_Runtime();			//contains WeaponID and other basic runtime data for both weapons

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
		void Init_Weapon(FName WeaponID, int32 WeaponIndex, FPlayerLoadoutConfig_Weapon WeaponLoadout);
		UFUNCTION()
		void Init_WeaponMesh(TWeakObjectPtr<USkeletalMeshComponent>& WeaponMesh);
		UFUNCTION(BlueprintCallable)
		void Init_Attachment(FWeaponAttachmentState& RuntimeSlotState, FInfantryWeaponState& WeaponToApplyTo, EAttachmentSlot AttachmentSlot);
		UFUNCTION(BlueprintCallable)
		void ApplyAttachments(const FPlayerLoadoutConfig_Weapon& AttachmentsToApply, int32 WeaponIndex);
		UFUNCTION()
		void UpdateWeaponMesh(FName WeaponID, TWeakObjectPtr<USkeletalMeshComponent>& WeaponMeshComp);
		UFUNCTION(BlueprintCallable)
		void UpdateWeaponData(int32 WeaponIndex, FName WeaponID, FInfantryWeaponState WeaponState);
		UFUNCTION()
		void UpdateAttachment(FWeaponAttachmentState& RuntimeSlotState, FName AttachmentID, FName WeaponID, EAttachmentSlot AttachmentSlot);
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
		UFUNCTION(BlueprintCallable, BlueprintPure)
		FWeapon_Runtime& GetBaseWeaponState(int32 WeaponIndex);
		UFUNCTION(BlueprintCallable, BlueprintPure)
		FVector GetAttachmentDefaultOffset(FName WeaponID, EAttachmentSlot Slot, FName AttachmentID);

	protected:
		TArray<const FInfantryWeaponData*> StaticWeaponDataCache;

		FWeapon_Runtime* GetCurrentWeaponRuntime();
		const FInfantryWeaponData* GetCurrentWeaponStaticData() const;

	private:
		UProjectilePoolSubsystem* ProjectileManager;
};
