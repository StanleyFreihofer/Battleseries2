#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Data_Gadget.generated.h"


/** 
 * data FOR CLASSIC GADGETS ONLY
 * gadgets that are weapons or vehicles in some manner should be listed in their respective data
**/

USTRUCT(BlueprintType)
struct FGadgetData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DisplayName = FText();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Description = FText();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ToolTip = "mesh that you hold"))
	TSoftObjectPtr<UStaticMesh> GadgetMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxActiveInstances = 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "if true, gadgets 'ammo' count will automatically start replenishing"))
	bool AutoRefill = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "if true, will automatically drop gadget on equip"))
	bool AutoDrop = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "if true, this gadget is able to be picked back when placed"))
	bool AbleToPickup = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftClassPtr<AActor> PlacedActorClass = nullptr;
};
