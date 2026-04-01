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

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText OpticDescription = FText();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (tooltip = "what color to turn UI when optic is on"))
	FLinearColor InverseUIColor = FLinearColor();

	//optic position (on, off, magnified, unmagnified)?

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FPostProcessSettings OpticPPSettings = FPostProcessSettings();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (tooltip = "the number to divide by (by itself represents the magnification value)")) //the number to divide current/default zoom by (by itself represents magnification value)
	float ZoomMagnification = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* PowerOnSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* PowerOffSound = nullptr;
};