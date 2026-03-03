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

//can either be a vehicle HMD (player screen) or HUD (worldspace widget component)

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

	UFUNCTION(BlueprintCallable)
	void UpdateSpeedometer(float Speed);

	UFUNCTION(BlueprintCallable)
	void UpdateCompassHUD(float Yaw);

	UFUNCTION(BlueprintCallable)
	void UpdateRangefinderHUD(float NewRange);

	UFUNCTION(BlueprintCallable)
	void UpdateWeaponNameHUD(FText WeaponDisplayName);
	UFUNCTION(BlueprintCallable)
	void UpdateWeaponStatusHUD(bool canFire);
	UFUNCTION(BlueprintCallable)
	void UpdateWeaponReticleHUD(UTexture2D* ImageBrush);
	UFUNCTION(BlueprintCallable)
	void UpdateWeaponReticleSize(float NewScale);
	UFUNCTION(BlueprintCallable)
	void UpdateEquippedWeaponHUD(FText WeaponName, UTexture2D* Reticle, float ReticleScale, bool canFire);
	
};