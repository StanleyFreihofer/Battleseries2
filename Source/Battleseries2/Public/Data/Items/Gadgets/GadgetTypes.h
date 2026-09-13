#pragma once

#include "CoreMinimal.h"
#include "GadgetTypes.generated.h"

USTRUCT(BlueprintType)
struct FGadgetState
{
	//struct for the character
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FName GadgetID = NAME_None;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (ToolTip = "current number of this gadget at runtime"))
	int32 CurrentInventory = 0;
	
	UPROPERTY(VisibleAnywhere, meta = (ToolTip = "any non-weapon gadget including vehicles like drones and eod bot's should be cached here"))
	TArray<TWeakObjectPtr<AActor>> ActivePlacedInstances;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TWeakObjectPtr<UStaticMeshComponent> HeldMesh_FP = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TWeakObjectPtr<UStaticMeshComponent> HeldMesh_TP = nullptr;
};

USTRUCT(BlueprintType)
struct FGadgetInstanceState
{
	//struct for the gadget itself
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UShapeComponent> TriggerVolume = nullptr;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 CurrentNumOfUses = 0;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)			//timer used regulate automated self-uses (resupply/give health every x seconds)
	FTimerHandle GadgetTickTimer = FTimerHandle();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)			//used if gadget has hard time limit
	FTimerHandle GadgetTimeLimitTimer = FTimerHandle();
};
