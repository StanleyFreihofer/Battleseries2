#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Data/Items/ItemEnums.h"
#include "Data_Gadget.generated.h"

USTRUCT(BlueprintType)
struct FGadgetData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DisplayName = FText();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Description = FText();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxActiveInstances = 1;
};
