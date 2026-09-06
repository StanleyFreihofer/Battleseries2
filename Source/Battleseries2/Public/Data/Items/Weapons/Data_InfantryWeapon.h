#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Data/Items/ItemStructs.h"
#include "Data/Items/Weapons/WeaponEnums.h"
#include "Data/Items/ItemEnums.h"
#include "Data/Items/Weapons/Data_Weapon.h"
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

#pragma region Recoil

USTRUCT(BlueprintType)
struct FControllerRecoilData
{
	GENERATED_BODY();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UCurveFloat* PitchControllerRecoil = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UCurveFloat* YawControllerRecoil = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float HipfireControllerRecoilMultiplier = 0.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ThirdPersonControllerRecoilMultiplier = 0.0f;
};

USTRUCT(BlueprintType)
struct FIKProceduralRecoilData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector MinLocation = FVector();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector MaxLocation = FVector();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector MinRotation = FVector();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector MaxRotation = FVector();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RecoilRecoverSpeed = 0.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float HipfireLocationMultiplier = 0.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float HipfireRotationMultiplier = 0.0f;
};

USTRUCT(BlueprintType)
struct FCrosshairRecoilData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CrosshairMovementSpeed = 0.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CrosshairFiringSpread = 0.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CrosshairAimingSpreadReducer = 0.0f;
};

USTRUCT(BlueprintType)
struct FInfantryWeaponRecoilData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FControllerRecoilData ControllerRecoilData = FControllerRecoilData();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FIKProceduralRecoilData IKProceduralRecoilData = FIKProceduralRecoilData();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FCrosshairRecoilData CrosshairRecoilData = FCrosshairRecoilData();
};

#pragma endregion

USTRUCT(BlueprintType)
struct FInfantryWeaponAmmoData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAmmoData BaseAmmoData = FAmmoData();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (tooltip = "if the weapon mesh has a projectile/munition as part of it, this will hide unless reloading"))
	FName ProjectileBoneToHide = NAME_None;

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
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FHeldItemAnimData_Base BaseItemAnimData = FHeldItemAnimData_Base();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Actions | Montages")
	TSoftObjectPtr<UAnimMontage> InspectMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Actions | Montages")
	TSoftObjectPtr<UAnimMontage> ReloadWeaponMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Actions | Montages")
	TSoftObjectPtr<UAnimMontage> ReloadEmptyWeaponMontage = nullptr;

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
	FInfantryWeaponRecoilData WeaponRecoilData = FInfantryWeaponRecoilData();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FInfantryWeaponAimData InfantryWeaponAimData = FInfantryWeaponAimData();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FInfantryWeaponAmmoData InfantryWeaponAmmoData = FInfantryWeaponAmmoData();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FInfantryWeaponAnimData InfantryWeaponAnimData = FInfantryWeaponAnimData();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGunAttachmentData GunAttachmentData = FGunAttachmentData();
};

