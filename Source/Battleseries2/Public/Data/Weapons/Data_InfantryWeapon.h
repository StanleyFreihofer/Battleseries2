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
	TMap<EAttachmentSlot, FAvailableAttachments> AvailableAttachmentSlots;
};

