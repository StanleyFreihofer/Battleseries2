#pragma once

#include "CoreMinimal.h"
#include "Core/UI/VehicleHUDs/UW_VehicleHUDComp_Base.h"
#include "UW_VehicleHUDComp_TurretBox.generated.h"

UCLASS()
class BATTLESERIES2_API UUW_VehicleHUDComp_TurretBox : public UUW_VehicleHUDComp_Base
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UImage* Img_OrientationMarker = nullptr;

	UFUNCTION(BlueprintCallable)
	void UpdateOrientationPosition(float Rotation, float Pitch);
};