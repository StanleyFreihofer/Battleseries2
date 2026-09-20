#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_HUD_Status_Base.generated.h"

UCLASS()
class BATTLESERIES2_API UUW_HUD_Status_Base : public UUserWidget
{
	GENERATED_BODY()

public:
	//weapon
	//ammo
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* T_CurrentAmmoInMag;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* T_CurrentReserveAmmo;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* T_CurrentFireMode;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* T_Auto;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* T_Burst;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* T_Single;
	
	//soldier health
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* T_HealthValue;
	
	//vehicle health
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UBorder* B_VehicleStatus;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* T_VehicleHealthValue;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UProgressBar* PB_VehicleHealthBar;

	UFUNCTION(BlueprintCallable)
	void UpdateCAMCount(int32 CAM);
	UFUNCTION(BlueprintCallable)
	void UpdateCRACount(int32 CRA);
	UFUNCTION(BlueprintCallable)
	void UpdateCurrentFireMode(EFireMode FireMode);
	UFUNCTION(BlueprintCallable)
	void UpdateCanFireModes(TArray<bool> canFireModes);		
	
	UFUNCTION(BlueprintCallable)
	void UpdateVehicleStatusVisibility(bool Hide);
	UFUNCTION(BlueprintCallable)
	void UpdateVehicleHealth(float NewHealth);
	
private:
	float FireModeNonActiveOpacity = 0.45f;				//<---MOVE THIS TO SOMEWHERE MORE INTENTIONAL/DATA ORIENTED
	
};
