#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Data/Items/Gadgets/GadgetEnums.h"
#include "Data_Gadget.generated.h"


/** 
 * data FOR CLASSIC GADGETS AND VEHICLE GADGETS ONLY
 * gadgets that are weapons should be listed in their respective data
 * WEAPON GADGETS SHOULD NOT BE LISTED HERE
**/

USTRUCT(BlueprintType)
struct FGadgetAnimData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UAnimMontage> EquipGadget = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UAnimMontage> UnequipGadget = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UAnimMontage> DeployGadget = nullptr;
};

USTRUCT(BlueprintType)
struct FGadgetData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DisplayName = FText();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Description = FText();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EGadgetType GadgetType = EGadgetType::Gadget;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "if the gadget is a vehicle or weapon, this is the ID for their respective DT", EditCondition = "GadgetType == EGadgetType::Vehicle || GadgetType == EGadgetType::Weapon", EditConditionHides))
	FName ItemID = NAME_None;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "mesh that you hold"))
	TSoftObjectPtr<UStaticMesh> GadgetMesh = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGadgetAnimData GadgetAnimData = FGadgetAnimData();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "if true, will automatically drop gadget on equip"))
	bool AutoDrop = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "if true, this gadget is able to be picked back when placed"))
	bool AbleToPickup = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "if true, this gadget will automatically be 'used' (if c4, auto detonate, if rc vehicle, auto start controlling)"))
	bool AutoUse = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "the gadget class/object that will be placed in the world"))
	TSoftClassPtr<AActor> PlacedActorClass = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "the default amount of the gadget a person gets"))					//same as max active instances?		//max inventory count?
	int32 DefaultInventoryCount = 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "if true, gadgets 'ammo' count will automatically start replenishing"))
	bool AutoRefill = false;
	
	//replenish delay?
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (TooTip = "the maximum amount of gadget entities/objects that can be placed/active in the world at once. If exceeded the first placed gadget will be deleted and removed from the world"))
	int32 MaxActiveInstances = 1;
};
