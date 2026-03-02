#pragma once

#include "CoreMinimal.h"
#include "Core/UI/VehicleHUDs/UW_VehicleHUDComp_Base.h"
#include "UW_VehicleHUDComp_Reticle.generated.h"

UCLASS()
class BATTLESERIES2_API UUW_VehicleHUDComp_Reticle : public UUW_VehicleHUDComp_Base
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UImage* Reticle = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UScaleBox* ScaleBox_Reticle = nullptr;

	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable)
	void UpdateReticleImage(UTexture2D* ImageBrush);
	UFUNCTION(BlueprintCallable)
	void UpdateReticleScale(float NewScale);
	UFUNCTION(BlueprintCallable)
	void UpdateReticlePosition(FVector2D NewPosition);

};