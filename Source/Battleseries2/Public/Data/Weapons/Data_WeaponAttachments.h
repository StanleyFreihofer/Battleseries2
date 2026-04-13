#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Data/Core/CoreTypes.h"
#include "Data_WeaponAttachments.generated.h"

/**
* all static data related to weapon attachments
**/

UENUM(BlueprintType)
enum class EWeaponAttachmentType : uint8
{
	Sight			UMETA(DisplayName = "Sight/Optic/Scope"),
	LaserLight		UMETA(DisplayName = "Laser/Light"),
	Handguard		UMETA(DisplayName = "Handguard"),
	Foregrip		UMETA(DisplayName = "Foregrip"),
	PistolGrip		UMETA(DisplayName = "Pistol Grip"),
	Underbarrel		UMETA(DisplayName = "Underbarrel"),
	Muzzle			UMETA(DisplayName = "Muzzle/Suppressor"),
	Stock			UMETA(DisplayName = "Stock"),
	RailCover		UMETA(DisplayName = "RailCover"),
	Mag				UMETA(DisplayName = "Magazine")
};

UENUM(BlueprintType)
enum class ESightSlot : uint8
{
	FrontSight		UMETA(DisplayName = "Front Sight"),
	RearSight		UMETA(DisplayName = "Rear Sight"),
	Scope			UMETA(DisplayName = "Optic/Scope"),
	Canted			UMETA(DisplayName = "Canted/Flip/Hybrid")
};

UENUM(BlueprintType)
enum class ETuningCapability : uint8
{
	NoTuning	UMETA(DisplayName = "No tuning"),
	StatOnly	UMETA(DisplayName = "Stat Only/No Visual"),
	Visual		UMETA(DisplayName = "Visual & Stat Affected")
};

/**
* STATIC
**/
USTRUCT(BlueprintType)
struct FWeaponSightData 
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ESightSlot SightSlot = ESightSlot::FrontSight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FPostProcessSettings SightPPSettings = FPostProcessSettings();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (tooltip = "the number to divide by (by itself represents the magnification value)")) //the number to divide current/default zoom by (by itself represents magnification value)
	float ZoomMagnification = 1.0f;
};

USTRUCT(BlueprintType)
struct FWeaponAttachmentClassification
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText AttachmentDisplayName = FText::GetEmpty();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText AttachmentDescription = FText::GetEmpty();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EWeaponAttachmentType WeaponAttachmentType = EWeaponAttachmentType::Sight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UStaticMesh> AttachmentMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> AttachmentIcon = nullptr;
};

USTRUCT(BlueprintType)
struct FAttachmentTuningData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ETuningCapability TuningCapability = ETuningCapability::NoTuning;

	// --- PHYSICAL MOVEMENT ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rail Tuning", meta = (EditCondition = "TuningCapability == ETuningCapability::Visual", EditConditionHides))
	float MaxRailForwardOffset = 15.0f; // Max cm it can move forward

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rail Tuning", meta = (EditCondition = "TuningCapability == ETuningCapability::Visual", EditConditionHides))
	float MaxRailBackwardOffset = -5.0f; // Max cm it can move back

	// --- STAT INFLUENCE ---
	//what stat does it influence (positively) (e.g., +10% control)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tuning Impact", meta = (EditCondition = "TuningCapability != ETuningCapability::NoTuning", EditConditionHides))
	FStatModifier TuningModifier = FStatModifier();

	//what stat does it influence (negatively) (e.g., +15% time)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tuning Impact", meta = (EditCondition = "TuningCapability != ETuningCapability::NoTuning", EditConditionHides))
	FStatModifier TuningPenalty = FStatModifier();
};

USTRUCT(BlueprintType)
struct FWeaponAttachmentData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FWeaponAttachmentClassification AttachmentClassification = FWeaponAttachmentClassification();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "AttachmentClassification.WeaponAttachmentType == EWeaponAttachmentType::Sight", EditConditionHides))
	FWeaponSightData WeaponSightData = FWeaponSightData();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (tooltip = "the stat modifiers (positive or negative) of this attachment"))
	TArray<FStatModifier> AttachmentModifiers;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (tooltip = "the tuning modifiers (positive/negative) for this attachment (modified on top of the base attachment modifiers)"))
	FAttachmentTuningData TuningModifier = FAttachmentTuningData();
};
