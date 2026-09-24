#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "HUDSubsystem.generated.h"

class UUW_HUD_Status_Base;
class UUW_HUD_Vehicle_Base;
class UUW_HUD_LockOnIndicator_Base;
class AVehicle_Base;
class UUW_Customization;

//management of all player screen widgets done here... HMD in practice
//component/worldspace widget done elsewhere
//****alot of stuff here needs to reflect the idea that we're now separating the idea of HMD and HUD****

UCLASS(Blueprintable, BlueprintType)
class BATTLESERIES2_API UHUDSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<UUW_HUD_Status_Base> StatusHUD = nullptr;         //ammo, health

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<UUW_HUD_Vehicle_Base> CurrentVehicleHMD = nullptr;        //standard HUD

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<UUW_HUD_LockOnIndicator_Base> LockOnIndicator = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UUserWidget> ScopeWidget = nullptr;
    //UPROPERTY(EditAnywhere, BlueprintReadWrite)
    //spawn UI

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "HUD | Customization")
    TObjectPtr<UUW_Customization> CustomizationWidget = nullptr;

    //killfeed
    //onfootreticle
    //minimap
    //scorefeed
	
#pragma region HUDSpawning
	
    UFUNCTION(BlueprintCallable)
    void SpawnStatusHUD(TSubclassOf<UUserWidget> HUDClass);
    UFUNCTION(BlueprintCallable)
    void SpawnVehicleSeatHUD(TSubclassOf<UUserWidget> HUDClass);
    UFUNCTION(BlueprintCallable)
    void SpawnLockOnIndicator(TSubclassOf<UUserWidget> HUDClass);
	UFUNCTION(BlueprintCallable)
	void SpawnScopeHUD(TSubclassOf<UUserWidget> HUDClass);
    UFUNCTION(BlueprintCallable)
    void SpawnCustomizationUI(TSubclassOf<UUserWidget> HUDClass);
	
#pragma endregion 
	
    UFUNCTION(BlueprintCallable)
    void UpdateLockOnIndicatorPosition(FVector Location);
    UFUNCTION(BlueprintCallable)
    void UpdateLockOnIndicatorStatus(ELockOnState LockOnState);
	
#pragma region Status HUD Updates
    UFUNCTION(BlueprintCallable)
    void UpdateStatusHUD_CAMCount(int32 CAM);
    UFUNCTION(BlueprintCallable)
    void UpdateStatusHUD_CRACount(int32 CRA);
	UFUNCTION(BlueprintCallable)
	void UpdateStatusHUD_FireMode(EFireMode FireMode);
	UFUNCTION(BlueprintCallable)
	void UpdateStatusHUD_canFireModes(TArray<bool> canFireModes);
	UFUNCTION(BlueprintCallable)
	void UpdateStatusHUD_VehicleStatusVisibility(bool Hide);
	UFUNCTION(BlueprintCallable)
	void UpdateStatusHUD_VehicleHealth(float NewHealth);
	
#pragma endregion 
	
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
    void UpdateOpticNameHUD_Vehicle(FText OpticDisplayName);
    UFUNCTION(BlueprintCallable)
    void UpdateOpticMagnificationHUD_Vehicle(float OpticMagnification);
    UFUNCTION(BlueprintCallable)
    void UpdateAltitudeHUD_Vehicle(float Pitch);

    UFUNCTION(BlueprintCallable)
    void RemoveWidget(UUW_HUD_LockOnIndicator_Base* UserWidget);

    UFUNCTION(BlueprintCallable)
    void UpdateVehicleHUD_Color(FLinearColor NewColor);

    UFUNCTION(BlueprintCallable)
    void RemoveCustomizationWidget();
};