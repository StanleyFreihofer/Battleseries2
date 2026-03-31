#include "Core/UI/VehicleHUDs/UW_HUD_Vehicle_Base.h"
#include "Core/UI/VehicleHUDs/UW_VehicleHUDComp_Base.h"
#include "Core/UI/VehicleHUDs/UW_VehicleHUDComp_Reticle.h"
#include "Core/UI/VehicleHUDs/UW_VehicleHUDComp_Rangefinder.h"
#include "Core/UI/VehicleHUDs/UW_VehicleHUDComp_WeaponStatus.h"
#include "Core/UI/VehicleHUDs/UW_VehicleHUDComp_Compass.h"
#include "Core/UI/VehicleHUDs/UW_VehicleHUDComp_TurretLines.h"
#include "Core/UI/VehicleHUDs/UW_VehicleHUDComp_TurretElvGauge.h"
#include "Core/UI/VehicleHUDs/UW_VehicleHUDComp_TurretPitchMeter.h"
#include "Core/UI/VehicleHUDs/UW_VehicleHUDComp_Optic.h"
#include "Core/UI/VehicleHUDs/UW_VehicleHUDComp_Speedometer.h"
#include "Utilities/I_HUDUpdates.h"
#include "Core/UI/VehicleHUDs/UW_HUD_Vehicle_Base.h"
#include "Blueprint/WidgetTree.h"

void UUW_HUD_Vehicle_Base::NativeConstruct()
{
	Super::NativeConstruct();
	BindComponents();
}

void UUW_HUD_Vehicle_Base::BindComponents()
{
	WidgetTree->ForEachWidget([&](UWidget* Widget)
	{
		if (UUW_VehicleHUDComp_Reticle* ReticleComp = Cast<UUW_VehicleHUDComp_Reticle>(Widget))
		{
			VehicleWeaponReticle = ReticleComp;
		}
		else if (UUW_VehicleHUDComp_WeaponStatus* StatusComp = Cast<UUW_VehicleHUDComp_WeaponStatus>(Widget))
		{
			VehicleWeaponStatus = StatusComp;
		}
		else if (UUW_VehicleHUDComp_Rangefinder* RangefinderComp = Cast<UUW_VehicleHUDComp_Rangefinder>(Widget))
		{
			Rangefinder = RangefinderComp;
		}
		else if (UUW_VehicleHUDComp_Compass* CompassComp = Cast<UUW_VehicleHUDComp_Compass>(Widget))
		{
			Compass = CompassComp;
		}
		else if (UUW_VehicleHUDComp_TurretLines* TurretLinesComp = Cast<UUW_VehicleHUDComp_TurretLines>(Widget))
		{
			TurretLines = TurretLinesComp;
		}
		else if (UUW_VehicleHUDComp_TurretElvGauge* TurretElvGaugeComp = Cast<UUW_VehicleHUDComp_TurretElvGauge>(Widget))
		{
			TurretElvGauge = TurretElvGaugeComp;
		}
		else if (UUW_VehicleHUDComp_TurretPitchMeter* TurretPitchMeterComp = Cast<UUW_VehicleHUDComp_TurretPitchMeter>(Widget))
		{
			TurretPitchMeter = TurretPitchMeterComp;
		}
		else if (UUW_VehicleHUDComp_Speedometer* SpeedometerComp = Cast<UUW_VehicleHUDComp_Speedometer>(Widget))
		{
			Speedometer = SpeedometerComp;
		}
		else if (UUW_VehicleHUDComp_Optic* OpticComp = Cast<UUW_VehicleHUDComp_Optic>(Widget))
		{
			OpticStatus = OpticComp;
		}
	});
}

void UUW_HUD_Vehicle_Base::UpdateSpeedometer(float Speed)
{
	if (Speedometer)
	{
		float DisplaySpeed = Speed * 0.036f;		// Unreal Units to KPH
		Speedometer->UpdateSpeedometer(DisplaySpeed);
	}
}

void UUW_HUD_Vehicle_Base::UpdateCompassHUD(float Yaw)
{
	if (Compass)
	{
		Compass->UpdateCompassPosition(Yaw);
	}
}

void UUW_HUD_Vehicle_Base::UpdateRangefinderHUD(float NewRange)
{
	if (Rangefinder)
	{
		Rangefinder->UpdateRangefinder(NewRange);
	}
}

void UUW_HUD_Vehicle_Base::UpdateWeaponNameHUD(FText WeaponDisplayName)
{
	if (VehicleWeaponStatus)
	{
		VehicleWeaponStatus->UpdateWeaponName(WeaponDisplayName);
	}
}

void UUW_HUD_Vehicle_Base::UpdateWeaponStatusHUD(bool canFire)
{
	if (VehicleWeaponStatus)
	{
		FText WeaponStatus;
		if (!canFire)
		{
			WeaponStatus = FText::FromString("WAIT");
		}
		else
		{
			WeaponStatus = FText::FromString("READY");
		}
		VehicleWeaponStatus->UpdateWeaponStatus(WeaponStatus);
	}
}

void UUW_HUD_Vehicle_Base::UpdateWeaponReticleHUD(UTexture2D* ImageBrush)
{
	if (VehicleWeaponReticle)
	{
		VehicleWeaponReticle->UpdateReticleImage(ImageBrush);
	}
}

void UUW_HUD_Vehicle_Base::UpdateWeaponReticleSize(float NewScale)
{
	if (VehicleWeaponReticle)
	{
		VehicleWeaponReticle->UpdateReticleScale(NewScale);
	}
}

void UUW_HUD_Vehicle_Base::UpdateEquippedWeaponHUD(FText WeaponName, UTexture2D* Reticle, float ReticleScale, bool canFire)
{
	//combine name and hud into 1 var?
	UpdateWeaponNameHUD(WeaponName);
	UpdateWeaponStatusHUD(canFire);

	UpdateWeaponReticleHUD(Reticle);
	UpdateWeaponReticleSize(ReticleScale);

}

void UUW_HUD_Vehicle_Base::UpdateOpticNameHUD(FText OpticName)
{
	if (OpticStatus)
	{
		OpticStatus->UpdateOpticName(OpticName);
	}
}

void UUW_HUD_Vehicle_Base::UpdateOpticMagnificationHUD(float OpticMagnification)
{
	if (OpticStatus)
	{
		OpticStatus->UpdateOpticMagnification(OpticMagnification);
	}
}

