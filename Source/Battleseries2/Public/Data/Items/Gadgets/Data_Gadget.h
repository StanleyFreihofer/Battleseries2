#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "CollisionShape.h"
#include "Data/Core/CoreEnums.h"
#include "Data/Core/CoreTypes.h"
#include "Data/Items/ItemStructs.h"
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
	FHeldItemAnimData_Base BaseItemAnimData = FHeldItemAnimData_Base();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UAnimMontage> DeployGadget = nullptr;
};

USTRUCT(BlueprintType)
struct FGadgetTriggerData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ECustomCollisionShapeType ShapeType = ECustomCollisionShapeType::Sphere;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "ShapeType == ECustomCollisionShapeType::Sphere"))
	float SphereRadius = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "ShapeType == ECustomCollisionShapeType::Box"))
	FVector BoxExtent = FVector(100.f);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "what type of thing is it triggered by"))
	TArray<ECoreObjectType> EligibleObjects;
	
	//offset (for claymores for example)
};

USTRUCT(BlueprintType)
struct FGadgetEffectData
{
	GENERATED_BODY()
	
	
	//1. ModifierOp (add, subtract, etc)
	//2. EffectValue (how to do non-predefined values (whats the current magsize size for the gun for example?)) 
	//3. thing to modify (would have to be state (maybe sometimes current stats)... health, armor, ammo, reload speed)
	//4. TickInterval (eg. every x seconds)
	//5. what things does it modify (characters, vehicle, projectile
	
	
	//EXAMPLE
	//ADD 30 AMMO 5 CHARACTER			(resupply 30 ammo to character every 5 seconds)
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EEffectCategory EffectCategory = EEffectCategory::StatEffect;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "EffectCategory == EEffectCategory::StatEffect", EditConditionHides, ToolTip = "each state this gadget affects, with its own operation/value/valuemode"))
	TMap<EStateToAffect, FStatModifierData> StateModifiers;
	
	//STAT modifiers (1 for vehicle, character, weapon, etc)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "0 = one-shot (claymore, mines); >0 = repeating (crates)"))
	float TickInterval = 0.f;		//initial vs. consecutive tick interval?
	
	//any further filters
};

USTRUCT(BlueprintType)
struct FGadgetInstanceData
{
	//Data that defines each instance of this gadget's function and behavior
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bHasTriggerVolume = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "bHasTriggerVolume", EditConditionHides))
	FGadgetTriggerData TriggerData = FGadgetTriggerData();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGadgetEffectData EffectData = FGadgetEffectData();
	
	//health/can be destroyed?
	//can be triggered via being shot or something?
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "time before gadget self-destructs, anything > 0 is considered an active value"))
	float TimeLimit = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "max amount of usages before gadget self-destructs, 0 is considered unlimited usages, 1 is essentially destroy on 1 use/trigger (eg. claymore)"))
	int32 MaxUsages = 0.f;

	//any other limits
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
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "Data that defines each instance of this gadget's function and behavior"))
	FGadgetInstanceData GadgetInstanceData = FGadgetInstanceData();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "if true, will automatically drop gadget on equip"))
	bool AutoDrop = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "if true, this gadget is able to be picked back up when placed"))
	bool AbleToPickup = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "if true, this gadget will automatically be 'used' (if c4, auto detonate, if rc vehicle, auto start controlling)"))
	bool AutoUse = false;
	
	//depreciate?
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
