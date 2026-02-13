#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "HUDSubsystem.generated.h"

class UUW_HUD_Status_Base;
class UUW_HUD_Vehicle_Base;
class UUW_HUD_LockOnIndicator_Base;
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UUW_HUD_LockOnIndicator_Base* LockOnIndicator;

    //killfeed
    //onfootreticle
    //minimap
    //scorefeed

    UFUNCTION(BlueprintCallable)
    void SpawnStatusHUD(TSubclassOf<UUserWidget> HUDClass);
    UFUNCTION(BlueprintCallable)
    void SpawnVehicleSeatHUD(TSubclassOf<UUserWidget> HUDClass);
    UFUNCTION(BlueprintCallable)
    void SpawnLockOnIndicator(TSubclassOf<UUserWidget> HUDClass);
    UFUNCTION(BlueprintCallable)
    void UpdateLockOnIndicatorPosition(FVector Location);
    UFUNCTION(BlueprintCallable)
    void UpdateStatusHUD_CAMCount(int32 CAM);
    UFUNCTION(BlueprintCallable)
    void UpdateStatusHUD_CRACount(int32 CRA);
    UFUNCTION(BlueprintCallable)
    void UpdateEquippedWeaponHUD_Vehicle(FText WeaponName, UTexture2D* Reticle, float ReticleScale, bool canFire);
    UFUNCTION(BlueprintCallable)
    void UpdateSpeedHUD_Vehicle(float Speed);
    UFUNCTION(BlueprintCallable)
    void UpdateWeaponReticleHUD_Vehicle(UTexture2D* ImageBrush);
    UFUNCTION(BlueprintCallable)
    void UpdateWeaponReticleSize_Vehicle(float NewScale);
    UFUNCTION(BlueprintCallable)
    void UpdateRangefinderHUD_Vehicle(float NewRange);
    UFUNCTION(BlueprintCallable)
    void UpdateWeaponNameHUD_Vehicle(FText WeaponDisplayName);
    UFUNCTION(BlueprintCallable)
    void UpdateWeaponStatusHUD_Vehicle(bool canFire);
    UFUNCTION(BlueprintCallable)
    void UpdateCompassHUD_Vehicle(float Yaw);
    UFUNCTION(BlueprintCallable)
    void UpdateTurretLinesHUD_Vehicle();
    UFUNCTION(BlueprintCallable)
    void UpdateTurretElevationHUD_Vehicle(float MinPitch, float MaxPitch, float CurrentPitch);
    UFUNCTION(BlueprintCallable)
    void HandleTurretRotationUpdate(float Yaw);
    UFUNCTION(BlueprintCallable)
    void HandleTurretPitchUpdate(float MinPitch, float MaxPitch, float CurrentPitch);

    UFUNCTION(BlueprintCallable)
    void RemoveWidget(UUW_HUD_LockOnIndicator_Base* UserWidget);
};