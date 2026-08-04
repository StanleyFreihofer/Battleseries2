#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Utilities/ProjectilePoolSubsystem.h"
#include "Utilities/DataManagerSubsystem.h"
#include "Data/Items/Weapons/WeaponTypes.h"
#include "Data/Items/Gadgets/GadgetTypes.h"
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
	FPlayerLoadoutConfig_WeaponAttachment BaseAttachmentState = FPlayerLoadoutConfig_WeaponAttachment();		//attachmentID, railoffset

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

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 CurrentOpticIndex = 0;
};

USTRUCT(BlueprintType)
struct FInfantryWeaponSystem
{
	//each array item corresponds to weapon index
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FInfantryWeaponState> WeaponState_FP;			//change to be more individual structs?

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FInfantryWeaponState> WeaponState_TP;			//change to be more individual structs?

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TArray<FWeaponStats_Runtime> CurrentWeaponStats;
};

USTRUCT(BlueprintType)
struct FOnFootWeaponSystem_Runtime
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FBaseWeaponSystem_Runtime BaseWeaponSystem = FBaseWeaponSystem_Runtime();			//contains WeaponID, Equipped WeaponState (CWI) and other basic runtime data for both weapons

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FInfantryWeaponSystem InfantryWeaponSystem = FInfantryWeaponSystem();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<USceneCaptureComponent2D> ScopeCamera = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool isAiming = false;
};

USTRUCT(BlueprintType) 
struct FLoadoutItemState 
{ 
	GENERATED_BODY() 
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)				//this 1 enum tells what category and slot/index 
	ELoadoutSlot CurrentSlot = ELoadoutSlot::PrimaryWeapon; 
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta= (ToolTip = "Weapon System, also contains any 'gadgets' that are Weapons"))				//contains "weapon" gadgets
	FOnFootWeaponSystem_Runtime WeaponSystem = FOnFootWeaponSystem_Runtime(); 
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite) 
	TArray<FGadgetState> Gadgets; 
	
	//grenade
	//melee
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 PreviousItemIndex = 0;
};

//RENAME TO LOADOUTMANAGER!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
UCLASS(Blueprintable, BlueprintType)
class BATTLESERIES2_API UWeaponLogicComponent : public UActorComponent
{
	GENERATED_BODY()

	public:
		UWeaponLogicComponent();
		virtual void BeginPlay() override;
		virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Runtime")
		FLoadoutItemState Loadout = FLoadoutItemState();

		UFUNCTION(BlueprintCallable)
		void Init_Loadout(TArray<FName> Weapons, TArray<FPlayerLoadoutConfig_Weapon> WeaponLoadouts, TArray<FName> Gadgets);
		UFUNCTION(BlueprintCallable)
		void Init_WeaponLoadout(TArray<FName> Weapons, TArray<FPlayerLoadoutConfig_Weapon> WeaponLoadouts);
		UFUNCTION(BlueprintCallable)
		void Init_GadgetLoadout(TArray<FName> Gadgets);
		UFUNCTION(BlueprintCallable)
		void Init_ScopeCamera();
		UFUNCTION(BlueprintCallable)
		void Init_Weapon(FName WeaponID, int32 WeaponIndex, FPlayerLoadoutConfig_Weapon WeaponLoadout);
		UFUNCTION()
		void Init_WeaponMesh(TWeakObjectPtr<USkeletalMeshComponent>& WeaponMesh);
		UFUNCTION(BlueprintCallable)
		void Init_Attachment(FWeaponAttachmentState& RuntimeSlotState, FInfantryWeaponState& WeaponToApplyTo, EAttachmentSlot AttachmentSlot);
		UFUNCTION(BlueprintCallable)
		void Init_Gadget(FName GadgetID, int32 GadgetIndex);
		UFUNCTION()
		void Init_GadgetMesh(TWeakObjectPtr<UStaticMeshComponent>& HeldGadgetMesh);
		UFUNCTION(BlueprintCallable)
		void SetupCustomWeapon(int32 WeaponIndex, FPlayerLoadoutConfig_Weapon WeaponLoadout);
		UFUNCTION(BlueprintCallable)
		void ApplyAttachments(const FPlayerLoadoutConfig_Weapon& AttachmentsToApply, int32 WeaponIndex);
		UFUNCTION()
		void UpdateWeaponMesh(FName WeaponID, TWeakObjectPtr<USkeletalMeshComponent>& WeaponMeshComp);
		UFUNCTION()
		void UpdateGadgetMesh(FName GadgetID, TWeakObjectPtr<UStaticMeshComponent>& GadgetMeshComp);
		UFUNCTION(BlueprintCallable)
		void UpdateWeaponData(int32 WeaponIndex, FName WeaponID, FInfantryWeaponState WeaponState);
		UFUNCTION(BlueprintCallable)
		void UpdateScopeCamera();
		UFUNCTION()
		void UpdateAttachment(FWeaponAttachmentState& RuntimeSlotState, FName AttachmentID, FName WeaponID, EAttachmentSlot AttachmentSlot);
		UFUNCTION(BlueprintCallable)
		void UpdateWeaponCollision(ECollisionChannel CollisionChannel, ECollisionResponse CollisionResponse, int32 WeaponIndex);
		UFUNCTION(BlueprintCallable)
		void UpdateWeaponVisibility(int32 WeaponIndex, bool Hide);
		UFUNCTION(BlueprintCallable)
		void StartAim();
		UFUNCTION(BlueprintCallable)
		void StopAim();
		UFUNCTION(BlueprintCallable)
		void Rangefinder();
		UFUNCTION(BlueprintCallable) 
		void HandleStartFire();
		UFUNCTION(BlueprintCallable)
		void StartFire();
		UFUNCTION(BlueprintCallable)
		void HandleShootProjectileActor();
		UFUNCTION(BlueprintCallable)
		void CeaseFire();
		UFUNCTION(BlueprintCallable)
		void DryFire();
		UFUNCTION(BlueprintCallable)
		void FireWeapon();
		UFUNCTION(BlueprintCallable)
		void ReloadWeapon();
		UFUNCTION(BlueprintCallable)
		void AutoSwitchItem();
		UFUNCTION(BlueprintCallable)
		void HandleSwitchItem(ELoadoutSlot NewLoadoutSlot);
		UFUNCTION(BlueprintCallable)
		void TransitionFromWeapon(int32 PreviousWeaponIndex);
		UFUNCTION(BlueprintCallable)
		void TransitionToItem();
		UFUNCTION(BlueprintCallable)
		void UnequipWeapon(int32 PreviousWeaponIndex);
		UFUNCTION(BlueprintCallable)
		void EquipWeapon(int32 WeaponIndex, bool InitalEquip);
		UFUNCTION(BlueprintCallable)
		void ToggleFireMode();
		UFUNCTION(BlueprintCallable)
		void ToggleScope();
		UFUNCTION(BlueprintCallable)
		void UpdateScope(int32 NewOpticIndex);
		UFUNCTION(BlueprintCallable)
		void UpdateCurrentWeaponStats(int32 WeaponIndex);
		UFUNCTION(BlueprintCallable)
		void ApplyAttachmentModifier(FWeaponStats_Runtime& RuntimeStats, EWeaponStat WeaponStat, const FStatModifierData& Modifier);
		UFUNCTION(BlueprintCallable)
		float CalculateFinalStatValue(float BaseValue, TArray<FStatModifierData>& ModifierArray);
	
		UFUNCTION(BlueprintCallable)
		void UnequipGadget(TSoftObjectPtr<UAnimMontage> FPUnequipItemMontage);
		UFUNCTION(BlueprintCallable)	
		void EquipGadget(int32 GadgetIndex);

	
		UFUNCTION(BlueprintCallable, BlueprintPure)
		ECharacterItemType GetCategoryForSlot(ELoadoutSlot LoadoutSlot);
		UFUNCTION(BlueprintCallable, BlueprintPure)
		int32 GetArrayIndex(ELoadoutSlot LoadoutSlot);
		UFUNCTION(BlueprintCallable, BlueprintPure)
		int32 GetWeaponIndexForSlot(ELoadoutSlot LoadoutSlot);
		UFUNCTION(BlueprintCallable, BlueprintPure)
		ECharacterItemType GetActualGadgetItemType(int32 GadgetIndex);
		UFUNCTION(BlueprintCallable, BlueprintPure)
		FName GetSocketNameForSlot(EAttachmentSlot Slot);
		UFUNCTION(BlueprintCallable, BlueprintPure)
		FTransform GetSightTransform();
		UFUNCTION(BlueprintCallable, BlueprintPure)
		float GetSightDistance();
		UFUNCTION(BlueprintCallable, BlueprintPure)
		void GetAimSpeeds(float& AimInSpeed, float& AimOutSpeed);
		UFUNCTION(BlueprintCallable, BlueprintPure)
		void GetAllAttachmentModifierDataOfTypeForWeapon(TMap<EAttachmentSlot, FWeaponAttachmentState>& WeaponAttachmentStates, EWeaponStat WeaponStatType, TArray<FStatModifierData>& OutAttachmentModifierData);
		UFUNCTION(BlueprintCallable, BlueprintPure)
		FWeaponAttachmentState& GetCurrentAttachmentInSlot(EAttachmentSlot Slot);
		UFUNCTION(BlueprintCallable, BlueprintPure)
		FWeapon_Runtime& GetBaseWeaponState(int32 WeaponIndex);
		UFUNCTION(BlueprintCallable, BlueprintPure)
		FInfantryWeaponState& GetCurrentInfantryWeaponState_FP();
		UFUNCTION(BlueprintCallable, BlueprintPure)
		FVector GetAttachmentDefaultOffset(FName WeaponID, EAttachmentSlot Slot, FName AttachmentID);
		UFUNCTION(BlueprintCallable, BlueprintPure)
		FInfantryWeaponData GetCurrentWeaponStaticData_BP();
		UFUNCTION(BlueprintCallable, BlueprintPure)
		FWeaponStats_Runtime& GetCurrentWeaponStats();
		UFUNCTION(BlueprintCallable, BlueprintPure)
		int32 GetCII();

	protected:
		TArray<const FInfantryWeaponData*> StaticWeaponDataCache;				//includes THE WEAPON DATA of gadgets that are weapons
		TArray<const FGadgetData*> StaticGadgetDataCache;

		FWeapon_Runtime* GetCurrentWeaponRuntime();
		const FInfantryWeaponData* GetCurrentWeaponStaticData();

		FTimerHandle SwitchWeaponTimer;

	private:
		void OnReloadFinished(UAnimMontage* Montage, bool bInterrupted);
		void OnUnequipWeapon_BlendOutToWeapon(UAnimMontage* Montage, bool bInterrupted);
		void OnUnequipWeapon_BlendOutToGadget(UAnimMontage* Montage, bool bInterrupted);
		FOnMontageEnded ReloadEndedDelegate;
		FOnMontageBlendingOutStarted UnequipBlendOutDelegate;
};


