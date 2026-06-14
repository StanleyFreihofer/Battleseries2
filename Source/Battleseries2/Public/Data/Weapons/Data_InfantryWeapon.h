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
struct FInfantryWeaponAmmoData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FAmmoData BaseAmmoData = FAmmoData();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bCanRoundBeChambered = true;
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
struct FInfantryWeaponAnimData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FInfantryWeaponAnimData_FP FPWeaponAnimData = FInfantryWeaponAnimData_FP();
};

USTRUCT(BlueprintType)
struct FAttachmentOffset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)				//data needed to move attachment to correct central/base location
	FVector LocationOffset = FVector::ZeroVector;
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
struct FInfantryWeaponData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FInfantryWeaponClassificationData WeaponClassificationData = FInfantryWeaponClassificationData();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FWeaponFunctionalityData WeaponFunctionalityData = FWeaponFunctionalityData();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FInfantryWeaponAmmoData InfantryWeaponAmmoData = FInfantryWeaponAmmoData();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FInfantryWeaponAnimData InfantryWeaponAnimData = FInfantryWeaponAnimData();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<EAttachmentSlot, FAvailableAttachments> AvailableAttachmentSlots;
};

