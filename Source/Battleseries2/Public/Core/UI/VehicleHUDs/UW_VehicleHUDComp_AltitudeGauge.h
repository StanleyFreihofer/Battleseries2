#pragma once

#include "CoreMinimal.h"
#include "Core/UI/VehicleHUDs/UW_VehicleHUDComp_Base.h"
#include "UW_VehicleHUDComp_AltitudeGauge.generated.h"

UCLASS()
class BATTLESERIES2_API UUW_VehicleHUDComp_AltitudeGauge : public UUW_VehicleHUDComp_Base
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UImage* Img_AltitudeGauge = nullptr;

	UFUNCTION(BlueprintCallable)
	void UpdateAltitudeGaugePosition(float Pitch);
};