//Data_Seat.h
//move this whole thing into vehicle?
#pragma once

#include "CoreMinimal.h"
#include "InputMappingContext.h"
#include "Components/SpotLightComponent.h"
#include "Data/Characters/CharacterEnums.h"
#include "Data/Weapons/WeaponEnums.h"
#include "Data/Vehicles/VehicleEnums.h"
#include "Data_Seat.generated.h"


USTRUCT(BlueprintType)
struct FWeaponCamBehavior
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EVehicleWeaponCamMountMethod MountMethod = EVehicleWeaponCamMountMethod::VehicleMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EVehicleWeaponCamActivationMethod WeaponCamActivationMethod = EVehicleWeaponCamActivationMethod::Equip;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName DefaultWeaponCamOptic = FName("O_60hz");
};

USTRUCT(BlueprintType)
struct FDecorative
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName SocketName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FTransform TransformOffset = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName AttachmentID = FName();
};

USTRUCT(BlueprintType)
struct FAttachmentInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Tooltip = "The mesh for this weapon if not just part of the vehicle mesh", EditCondition = "bHasSeparateMesh", EditConditionHides = true))
	FName AttachmentID = FName();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bHasSeparateMesh", EditConditionHides = true))
	FTransform AttachmentTransform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Tooltip = "whether or not to attach the character to a socket on this mesh, would allow the character to move with mesh, like a turret for example", EditCondition = "bHasSeparateMesh", EditConditionHides = true))
	bool bAttachCharacter = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Tooltip = "relative transform of character on this mesh if attached", EditCondition = "bAttachCharacter", EditConditionHides = true))
	FTransform CharacterTransform = FTransform::Identity;
};

USTRUCT(BlueprintType)
struct FWeaponUIInstanceData
{
	GENERATED_BODY()

	//reticle data
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UTexture2D* WeaponReticle = nullptr;		//russian reticle for tank shell in t72/t90, etc
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ReticleScale = 1.0f;
};


//move to weapon data/somewhere else?
USTRUCT(BlueprintType)
struct FVehicleWeaponInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EFireMethod FireMethod = EFireMethod::Default;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bAreProjectilesMounted = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EMuzzleType MuzzleType = EMuzzleType::Gun;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "determines what part of the vehicle's hierarchy determines the forward direction for this weapon, only matters for windowed seats"))
	EWindowedAimAnchor WindowedAimAnchor = EWindowedAimAnchor::Hull;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "Recoil multiplier that gets multiplied to angular impulse"))
	float RecoilMultiplier = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bHasSeparateMesh = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "bHasSeparateMesh", EditConditionHides = true))
	FAttachmentInstanceData AttachmentInstanceData = FAttachmentInstanceData();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bHasSpecialCam = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "bHasSpecialCam", EditConditionHides = true))
	FWeaponCamBehavior WeaponCamBehavior = FWeaponCamBehavior();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FWeaponUIInstanceData WeaponUIInstanceData = FWeaponUIInstanceData();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FDecorative> WeaponDecoratives;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FName> BonesToHide;
};

USTRUCT(BlueprintType)
struct FWeaponSlotChoices
{
	GENERATED_BODY()

	// This is the map that holds all weapon choices for a single slot.
	//this is its own struct because blueprints dont play nice with maps
	//key = WeaponID
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FName, FVehicleWeaponInstanceData> WeaponChoices;

	//bSlotThatDrivesWeaponMesh?
};

USTRUCT(BlueprintType)
struct FCountermeasureDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName CountermeasureID = NAME_None;
	//countermeasure instance data
};

USTRUCT(BlueprintType)
struct FOpticDefinition
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName OpticID = NAME_None;
};

USTRUCT(BlueprintType)
struct FUpgradeDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName UpgradeID = NAME_None;
	//upgrade instance data
};


USTRUCT(BlueprintType)
struct FAvailableItems
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (tooltip = "the weapon index that drives the weapon mesh for this seat"))
	int32 WeaponMeshDriverSlotIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FWeaponSlotChoices> AvailableWeaponSlots; //index 0 is primary, 1 is secondary

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (tooltip = "The Array Indexes of the turret(s) this seat controls"))
	TArray<int32> ControlledTurretIndexes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FCountermeasureDefinition> AvailableCountermeasures;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FName> AvailableOptics;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FUpgradeDefinition> AvailableUpgrades;

	TArray<FName> GetDefaultWeaponIDs() const
	{
		TArray<FName> DefaultWeaponIDs;

		for (const FWeaponSlotChoices& SlotWrapper : AvailableWeaponSlots)
		{
			const TMap<FName, FVehicleWeaponInstanceData>& SlotChoices = SlotWrapper.WeaponChoices;
			if (SlotChoices.Num() > 0)
			{
				auto MapIterator = SlotChoices.CreateConstIterator();
				DefaultWeaponIDs.Add(MapIterator.Key());
			}
			else
			{
				DefaultWeaponIDs.Add(NAME_None);
			}
		}
		return DefaultWeaponIDs;
	}
};

USTRUCT(BlueprintType)
struct FCharacterSeatContext
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsCharacterVisible = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (tooltip = "standard seat HUD/HMD (non-worldspace, added to player screen)"))
	TSubclassOf<UUserWidget> SeatHMD = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (tooltip = "actual in-word HUD (worldspace widget), leave this blank if none"))
	TSubclassOf<UUserWidget> SeatHUD = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (tooltip = "relative transform within the vehicle of the widget component", EditCondition = "SeatHUD != nullptr", EditConditionHides = true))
	FTransform SeatHUDTransform = FTransform();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "SeatHUD != nullptr", EditConditionHides = true))
	FVector2D SeatHUDDrawSize = FVector2D();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UInputMappingContext* InputMappingContext = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FTransform SeatTransform = FTransform();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ECharacterCurrentStance SeatStance = ECharacterCurrentStance::Sitting;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EControlRotationMethod CharacterRotationMethod = EControlRotationMethod::None;
};

USTRUCT(BlueprintType)
struct FSeatData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (tooltip = "also used for the attaching of player character to socket on vehicle mesh"))
	FText SeatName = FText::GetEmpty();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	E_SeatRole SeatRole = E_SeatRole::Driver;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	E_ViewMethod ViewMethod = E_ViewMethod::Windowed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FCharacterSeatContext DefaultCharacterContext = FCharacterSeatContext();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName DefaultOptic = FName("O_60hz");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "SeatRole == E_SeatRole::DriverGunner || SeatRole == E_SeatRole::Gunner", EditConditionHides))
	FAvailableItems AvailableItems;
};
