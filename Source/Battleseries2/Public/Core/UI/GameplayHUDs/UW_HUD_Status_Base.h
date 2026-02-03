#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_HUD_Status_Base.generated.h"

UCLASS()
class BATTLESERIES2_API UUW_HUD_Status_Base : public UUserWidget
{
	GENERATED_BODY()

public:
	//ammo
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* T_CurrentAmmoInMag;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* T_CurrentReserveAmmo;

	//soldier health
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* T_HealthValue;
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	//class UProgressBar* PB_HealthBar;

	//vehicle health
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* T_VehicleHealthValue;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UProgressBar* PB_VehicleHealthBar;

	UFUNCTION(BlueprintCallable)
	void UpdateCAMCount(int32 CAM);
	UFUNCTION(BlueprintCallable)
	void UpdateCRACount(int32 CRA);
	
};
