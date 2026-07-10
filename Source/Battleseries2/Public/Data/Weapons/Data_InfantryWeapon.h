#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Data/Weapons/WeaponEnums.h"
#include "Data/Core/CoreEnums.h"
#include "Data/Weapons/Data_Weapon.h"
#include "Data_InfantryWeapon.generated.h"

USTRUCT(BlueprintType)
struct FInfantryWeaponClassificationData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBaseWeaponClassificationData BaseWeaponClassificationData = FBaseWeaponClassificationData();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EWeaponType WeaponType = EWeaponType::AssaultRifle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> WeaponIcon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<USkeletalMesh> WeaponMesh = nullptr;
};

USTRUCT(BlueprintType)
struct FInfantryWeaponFunctionalityData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FWeaponFunctionalityData BaseWeaponFunctionality = FWeaponFunctionalityData();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool canADSReload = false;

	//canholdbreath
};

USTRUCT(BlueprintType)
struct FInfantryWeaponAmmoData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FAmmoData BaseAmmoData = FAmmoData();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bCanRoundBeChambered = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (tooltip = "is projectile mounted/loaded in gun (will show the actual projectile fired)"))
	bool isProjectileMounted = false;
};

USTRUCT(BlueprintType)
struct FInfantryWeaponAimData
{
	GENERATED_BODY()

	//cycle sight speed?

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool canAim = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "Hide First Person Arms when ADS?"))
	bool HideArms = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DefaultAimInSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DefaultAimOutSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DefaultSightDistance = 0.0f;
};

USTRUCT(BlueprintType)
struct FInfantryWeaponAnimData_FP
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Actions | Montages")
	TSoftObjectPtr<UAnimMontage> InspectMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Actions | Montages")
	TSoftObjectPtr<UAnimMontage> ReloadWeaponMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Actions | Montages")
	TSoftObjectPtr<UAnimMontage> ReloadEmptyWeaponMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Actions | Montages")
	TSoftObjectPtr<UAnimMontage> UnequipWeaponMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Actions | Montages")
	TSoftObjectPtr<UAnimMontage> EquipWeaponMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Actions | Montages")
	TSoftObjectPtr<UAnimMontage> InitialEquipWeaponMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Actions | Montages")
	TSoftObjectPtr<UAnimMontage> FireWeaponMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Actions | Montages")
	TSoftObjectPtr<UAnimMontage> DryFireMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transitions | Montages")
	TSoftObjectPtr<UAnimMontage> EnterReloadMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transitions | Montages")
	TSoftObjectPtr<UAnimMontage> ExitReloadMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transitions | Montages")
	TSoftObjectPtr<UAnimMontage> LoopReloadMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transitions | Sequences")
	TSoftObjectPtr<UAnimSequence> TacSprintExit = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transitions | Sequences")
	TSoftObjectPtr<UAnimSequence> TacSprintEnter = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transitions | Sequences")
	TSoftObjectPtr<UAnimSequence> SprintEnter = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transitions | Sequences")
	TSoftObjectPtr<UAnimSequence> SprintExit = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transitions | Sequences")
	TSoftObjectPtr<UAnimSequence> TacSprintLoopAdditive = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transitions | Sequences")
	TSoftObjectPtr<UAnimSequence> SlideEnterAdditive = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transitions | Sequences")
	TSoftObjectPtr<UAnimSequence> JumpEnterAdditive = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transitions | Sequences")
	TSoftObjectPtr<UAnimSequence> JumpExitAdditive = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transitions | Sequences")
	TSoftObjectPtr<UAnimSequence> TacSprintExitIdleAdditive = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transitions | Sequences")
	TSoftObjectPtr<UAnimSequence> ProneEnterAdditive = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transitions | Sequences")
	TSoftObjectPtr<UAnimSequence> ProneExitAdditive = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transitions | Sequences")
	TSoftObjectPtr<UAnimSequence> SlidingExit = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transitions | Sequences")
	TSoftObjectPtr<UAnimSequence> TacSprintEnterIdle = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transitions | Sequences")
	TSoftObjectPtr<UAnimSequence> CrouchEnter = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transitions | Sequences")
	TSoftObjectPtr<UAnimSequence> CrouchExit = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Regrip | Montages")
	TSoftObjectPtr<UAnimMontage> ShoulderRegrip = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Regrip | Montages")
	TSoftObjectPtr<UAnimMontage> Regrip = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Regrip | Montages")
	TSoftObjectPtr<UAnimMontage> RegripAlternate = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Regrip | Montages")
	TSoftObjectPtr<UAnimMontage> RegripADS = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Regrip | Sequences")
	TSoftObjectPtr<UAnimSequence> ReshoulderAdditive = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement | Blendspaces")
	TSoftObjectPtr<UBlendSpace> MovementBlendspace = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement | Blendspaces")
	TSoftObjectPtr<UBlendSpace> ProneBlendspace = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	TSoftObjectPtr<UAnimSequence> FallingLoop = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	TSoftObjectPtr<UAnimSequence> Idle = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	TSoftObjectPtr<UAnimSequence> JumpLoop = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	TSoftObjectPtr<UAnimSequence> SprintLoop = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	TSoftObjectPtr<UAnimSequence> SlideLoop = nullptr;
};

USTRUCT(BlueprintType)
struct FInfantryWeaponAnimData_Weapon
{
	//animations of the gun itself, not the character holding it
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UAnimSequence> WeaponFire = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UAnimSequence> WeaponReload = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UAnimSequence> WeaponReloadEmpty = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UAnimSequence> WeaponReloadEnter = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UAnimSequence> WeaponReloadExit = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UAnimSequence> WeaponEquip = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UAnimSequence> WeaponInspect = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UAnimSequence> WeaponUnholsterInitial = nullptr;
};

USTRUCT(BlueprintType)
struct FInfantryWeaponAnimData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FInfantryWeaponAnimData_FP FPWeaponAnimData = FInfantryWeaponAnimData_FP();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FInfantryWeaponAnimData_Weapon WeaponAnimData = FInfantryWeaponAnimData_Weapon();
};

USTRUCT(BlueprintType)
struct FAttachmentOffset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)				//data needed to move attachment to correct central/base location
	FVector LocationOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRotator RotationOffset = FRotator::ZeroRotator;
};

USTRUCT(BlueprintType)
struct FAvailableAttachments
{
	GENERATED_BODY()

	//attachmentID->instance
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, FAttachmentOffset> Attachments;
};

USTRUCT(BlueprintType)
struct FGunAttachmentData
{
	GENERATED_BODY()
	//cycle sight speed?

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<EAttachmentSlot, FAvailableAttachments> AvailableAttachmentSlots;
};

USTRUCT(BlueprintType)
struct FInfantryWeaponData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FInfantryWeaponClassificationData WeaponClassificationData = FInfantryWeaponClassificationData();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FInfantryWeaponFunctionalityData WeaponFunctionalityData = FInfantryWeaponFunctionalityData();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FWeaponFirePerformanceData WeaponFirePerformanceData = FWeaponFirePerformanceData();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FInfantryWeaponAimData InfantryWeaponAimData = FInfantryWeaponAimData();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FInfantryWeaponAmmoData InfantryWeaponAmmoData = FInfantryWeaponAmmoData();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FInfantryWeaponAnimData InfantryWeaponAnimData = FInfantryWeaponAnimData();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGunAttachmentData GunAttachmentData = FGunAttachmentData();
};

