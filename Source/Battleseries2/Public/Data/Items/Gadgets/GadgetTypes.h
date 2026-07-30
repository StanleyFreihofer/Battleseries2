#pragma once

#include "CoreMinimal.h"
#include "GadgetTypes.generated.h"

USTRUCT(BlueprintType)
struct FGadgetState
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FName GadgetID = NAME_None;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (ToolTip = "current number of this gadget at runtime"))
	int32 CurrentInventory = 0;
	
	UPROPERTY(VisibleAnywhere)
	TArray<TWeakObjectPtr<AActor>> ActivePlacedInstances;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TWeakObjectPtr<UStaticMeshComponent> HeldMesh_FP = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TWeakObjectPtr<UStaticMeshComponent> HeldMesh_TP = nullptr;
};
