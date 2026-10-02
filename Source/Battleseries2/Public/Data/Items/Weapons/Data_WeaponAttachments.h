#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Data/Core/CoreTypes.h"
#include "Data/Core/CoreEnums.h"
#include "Data/Items/Weapons/WeaponEnums.h"
#include "Data/Items/Weapons/WeaponStructs.h"
#include "Data_WeaponAttachments.generated.h"

/**
* all static data related to weapon attachments
**/


/**
* STATIC
**/
USTRUCT(BlueprintType)
struct FWeaponSightData 
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ESightSlot SightSlot = ESightSlot::FrontSight;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "use to achieve black around scope effect, only added on screen when ADS"))
	TSubclassOf<UUserWidget> ScopeHUD = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (tooltip = "optic settings to pull from optic data table"))
	TArray<FName> OpticIDs;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (tooltip = "the default number to increase/decrease the sight distance by (helps to ensure when aim, sight distance brings camera to front of sight rather than center/in it"))
	float SightDistanceOffset = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (tooltip = "the number to increase/decrease the default SightTransform by, helps to ensure vertically we aim down center of sight"))
	float VerticalAimpointOffset = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 PIPMaterialIndex = -1;

	FWeaponSightData()
	{
		OpticIDs.Add(FName("O_60hz"));
	}
};

USTRUCT(BlueprintType)
struct FAmmoVisualElement
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EAmmoVisualBehavior Behavior = EAmmoVisualBehavior::Static;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "Behavior != EAmmoVisualBehavior::HideBone", EditConditionHides))
	TSoftObjectPtr<UStaticMesh> Mesh = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "spawns one per socket found: <Prefix>_0, _1 ... (or bone name for HideBone)"))
	FName SocketPrefix = NAME_None;	//make this more standardized rather than a standalone variable
};

USTRUCT(BlueprintType)
struct FMagazineData
{
	GENERATED_BODY()
	
	//override ammo visuals bool?
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "attach AttachmentMesh here instead of the Magazine slot's default socket (e.g. S_Mag_Side for M249 STANAG adapter). none = default"))
	FName SocketOverride = NAME_None;		//make this more standardized rather than a standalone variable
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "loads 1 round at a time (enter/loop/exit) vs whole mag. lives here since conversions change it (shotgun box mag)"))
	bool bIsBulletFed = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "all visible ammo/feed meshes for this feed (belt, rounds, casings, in-hand items)"))
	TArray<FAmmoVisualElement> AmmoVisuals;
	
	//any animation overrides
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
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "decorative mesh attached to weapon in addition to main mesh (mg belt for example), 1 is spawned and attached for each 'S_AttachmentSlot_Decorative_index'"))
	TSoftObjectPtr<UStaticMesh> DecorativeMesh = nullptr;
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
	TMap<EWeaponStat, FWeaponStatModifierData> TuningModifiers;

	//what stat does it influence (negatively) (e.g., +15% time)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tuning Impact", meta = (EditCondition = "TuningCapability != ETuningCapability::NoTuning", EditConditionHides))
	TMap<EWeaponStat, FWeaponStatModifierData> TuningPenalty;
};

USTRUCT(BlueprintType)
struct FWeaponAttachmentData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FWeaponAttachmentClassification AttachmentClassification = FWeaponAttachmentClassification();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "AttachmentClassification.WeaponAttachmentType == EWeaponAttachmentType::Sight", EditConditionHides))
	FWeaponSightData WeaponSightData = FWeaponSightData();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "AttachmentClassification.WeaponAttachmentType == EWeaponAttachmentType::Mag", EditConditionHides))
	FMagazineData MagazineData = FMagazineData();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "the stat modifiers (positive or negative) of this attachment"))
	TMap<EWeaponStat, FWeaponStatModifierData> AttachmentModifiers;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "the tuning modifiers (positive/negative) for this attachment (modified on top of the base attachment modifiers)"))
	FAttachmentTuningData TuningModifier = FAttachmentTuningData();
};
