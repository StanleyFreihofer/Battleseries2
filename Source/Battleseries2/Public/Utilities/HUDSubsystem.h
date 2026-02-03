#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "HUDSubsystem.generated.h"

class UUW_HUD_Status_Base;
class UUW_HUD_Vehicle_Base;
class AVehicle_Base;

UCLASS(Blueprintable, BlueprintType)
class BATTLESERIES2_API UHUDSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UUW_HUD_Status_Base* StatusHUD;         //ammo, health

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UUW_HUD_Vehicle_Base* CurrentVehicleHUD;

    //killfeed
    //onfootreticle
    //minimap
    //scorefeed
    //ammohealth

    UFUNCTION(BlueprintCallable)
    void SpawnStatusHUD(TSubclassOf<UUserWidget> HUDClass);
    UFUNCTION(BlueprintCallable)
    void SpawnVehicleSeatHUD(TSubclassOf<UUserWidget> HUDClass);
    UFUNCTION(BlueprintCallable)
    void SetupVehicleDriverHUD();
    UFUNCTION(BlueprintCallable)
    void SetupVehicleGunnerHUD();
    UFUNCTION(BlueprintCallable)
    void UpdateStatusHUD_CAMCount_Vehicle(int32 SeatIndex);
    UFUNCTION(BlueprintCallable)
    void UpdateEquippedWeaponHUD_Vehicle(int32 SeatIndex);
    UFUNCTION(BlueprintCallable)
    void UpdateSpeedHUD_Vehicle();
    UFUNCTION(BlueprintCallable)
    void UpdateWeaponReticleHUD_Vehicle(UTexture2D* ImageBrush);
    UFUNCTION(BlueprintCallable)
    void UpdateWeaponReticleSize_Vehicle(float NewScale);
    UFUNCTION(BlueprintCallable)
    void UpdateRangefinderHUD_Vehicle(AVehicle_Base* ReportingVehicle, int32 SeatIndex, float NewRange);
    UFUNCTION(BlueprintCallable)
    void UpdateWeaponNameHUD_Vehicle(FText WeaponDisplayName);
    UFUNCTION(BlueprintCallable)
    void UpdateWeaponStatusHUD_Vehicle(FText WeaponStatus);
    UFUNCTION(BlueprintCallable)
    void UpdateCompassHUD_Vehicle(int32 SeatIndex);
    UFUNCTION(BlueprintCallable)
    void UpdateTurretLinesHUD_Vehicle();
    UFUNCTION(BlueprintCallable)
    void UpdateTurretElevationHUD_Vehicle(int32 SeatIndex);
    UFUNCTION(BlueprintCallable)
    void HandleTurretRotationUpdate(int32 SeatIndex);
    UFUNCTION(BlueprintCallable)
    void HandleTurretPitchUpdate(int32 SeatIndex);
};