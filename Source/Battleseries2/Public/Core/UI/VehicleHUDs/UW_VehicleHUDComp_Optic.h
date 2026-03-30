#pragma once

#include "CoreMinimal.h"
#include "Core/UI/VehicleHUDs/UW_VehicleHUDComp_Base.h"
#include "UW_VehicleHUDComp_Optic.generated.h"

UCLASS()
class BATTLESERIES2_API UUW_VehicleHUDComp_Optic : public UUW_VehicleHUDComp_Base
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* T_OpticName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* T_OpticMagnification;
	UFUNCTION(BlueprintCallable)
	void UpdateOpticName(FText OpticName);
	UFUNCTION(BlueprintCallable)
	void UpdateOpticMagnification(float OpticMagnification);
};