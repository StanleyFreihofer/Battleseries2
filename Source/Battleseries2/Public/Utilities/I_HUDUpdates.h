#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "I_HUDUpdates.generated.h"

UINTERFACE(MinimalAPI)
class UHUDUpdates : public UInterface
{
	GENERATED_BODY()
};

class BATTLESERIES2_API IHUDUpdates
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "HUD Interface")
	void UpdateHUDColor(FLinearColor NewColor);
};
