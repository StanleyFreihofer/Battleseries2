#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Utilities/I_HUDUpdates.h"
#include "UW_VehicleHUDComp_Base.generated.h"

UCLASS()
class BATTLESERIES2_API UUW_VehicleHUDComp_Base : public UUserWidget, public IHUDUpdates
{
	GENERATED_BODY()

public:
	virtual void UpdateHUDColor_Implementation(FLinearColor NewColor) override;
};
