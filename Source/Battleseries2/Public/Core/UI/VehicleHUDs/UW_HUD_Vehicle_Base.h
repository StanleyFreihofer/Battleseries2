#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_HUD_Vehicle_Base.generated.h"

class UUW_VehicleHUDComp_Reticle;
class UUW_VehicleHUDComp_WeaponStatus;
class UUW_VehicleHUDComp_Rangefinder;
class UUW_VehicleHUDComp_Compass;
class UUW_VehicleHUDComp_TurretLines;
class UUW_VehicleHUDComp_TurretElvGauge;
class UUW_VehicleHUDComp_TurretPitchMeter;
class UUW_VehicleHUDComp_Speedometer;

UCLASS()
class BATTLESERIES2_API UUW_HUD_Vehicle_Base : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle HUD")
	UUW_VehicleHUDComp_Reticle* VehicleWeaponReticle = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle HUD")
	UUW_VehicleHUDComp_WeaponStatus* VehicleWeaponStatus = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle HUD")
	UUW_VehicleHUDComp_Rangefinder* Rangefinder = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle HUD")
	UUW_VehicleHUDComp_Compass* Compass = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle HUD")
	UUW_VehicleHUDComp_TurretLines* TurretLines = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle HUD")
	UUW_VehicleHUDComp_TurretElvGauge* TurretElvGauge = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle HUD")
	UUW_VehicleHUDComp_TurretPitchMeter* TurretPitchMeter = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle HUD")
	UUW_VehicleHUDComp_Speedometer* Speedometer = nullptr;
	//optic info
	//countermeasure status
	//warning (health, mobility, lockon)
	//altimeter/altimeter gauge
	//speed gauge

	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable)
	void BindComponents();
};