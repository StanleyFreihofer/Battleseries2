#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Data_Optics.generated.h"

USTRUCT(BlueprintType)
struct FOpticData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText OpticDisplayName = FText();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)		
	FText OpticDisplayNameAbrev = FText::FromString("60hz");

	//optic position (on, off, magnified, unmagnified)?

	//inverse UI color?

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FPostProcessSettings OpticPPSettings = FPostProcessSettings();

	UPROPERTY(EditAnywhere, BlueprintReadOnly) //the number to divide current/default zoom by (by itself represents magnification value)
	float ZoomMagnification = 1.0f;
};