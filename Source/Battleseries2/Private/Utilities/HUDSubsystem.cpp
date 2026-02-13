#include "Utilities/HUDSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Core/UI/GameplayHUDs/UW_HUD_Status_Base.h"
#include "Core/UI/GameplayHUDs/UUW_HUD_LockOnIndicator_Base.h"
#include "Core/UI/VehicleHUDs/UW_HUD_Vehicle_Base.h"
#include "Core/UI/VehicleHUDs/UW_VehicleHUDComp_Reticle.h"
#include "Core/UI/VehicleHUDs/UW_VehicleHUDComp_Rangefinder.h"
#include "Core/UI/VehicleHUDs/UW_VehicleHUDComp_WeaponStatus.h"
#include "Core/UI/VehicleHUDs/UW_VehicleHUDComp_Compass.h"
#include "Core/UI/VehicleHUDs/UW_VehicleHUDComp_TurretLines.h"
#include "Core/UI/VehicleHUDs/UW_VehicleHUDComp_TurretElvGauge.h"
#include "Core/UI/VehicleHUDs/UW_VehicleHUDComp_TurretPitchMeter.h"
#include "Core/UI/VehicleHUDs/UW_VehicleHUDComp_Speedometer.h"
#include "Data/Weapons/VehicleWeapons/Data_VehicleWeapon.h"
#include "Character_Base.h"
#include "Vehicle_Base.h"
#include "Core/Weapons/VehicleWeaponLogicComponent.h"

void UHUDSubsystem::SpawnStatusHUD(TSubclassOf<UUserWidget> HUDClass)
{
	APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	StatusHUD = CreateWidget<UUW_HUD_Status_Base>(PC, HUDClass);
	StatusHUD->AddToViewport();
}

void UHUDSubsystem::SpawnVehicleSeatHUD(TSubclassOf<UUserWidget> HUDClass)
{
	//called on enter seat
	APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());

	CurrentVehicleHUD = CreateWidget<UUW_HUD_Vehicle_Base>(PC, HUDClass);
	CurrentVehicleHUD->AddToViewport();

	//do everything at least once maybe?
}

void UHUDSubsystem::SpawnLockOnIndicator(TSubclassOf<UUserWidget> HUDClass)
{
	APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	LockOnIndicator = CreateWidget<UUW_HUD_LockOnIndicator_Base>(PC, HUDClass);
	LockOnIndicator->AddToViewport();
}

void UHUDSubsystem::UpdateLockOnIndicatorPosition(FVector Location)
{
	if (LockOnIndicator)
	{
		LockOnIndicator->UpdateIndicatorPosition(Location);
	}
}

void UHUDSubsystem::UpdateStatusHUD_CAMCount(int32 CAM)
{
	StatusHUD->UpdateCAMCount(CAM);
}

void UHUDSubsystem::UpdateStatusHUD_CRACount(int32 CRA)
{
	StatusHUD->UpdateCRACount(CRA);
}

void UHUDSubsystem::UpdateSpeedHUD_Vehicle(float Speed)
{
	if (CurrentVehicleHUD && CurrentVehicleHUD->Speedometer)
	{
		//float RawSpeed = Character->CharacterState.CharacterVehicleState.CurrentVehicle->GetVelocity().Size();
		//use chaos speed?
		float DisplaySpeed = Speed * 0.036f;		// Unreal Units to KPH
		CurrentVehicleHUD->Speedometer->UpdateSpeedometer(DisplaySpeed);
	}
}

void UHUDSubsystem::UpdateEquippedWeaponHUD_Vehicle(FText WeaponName, UTexture2D* Reticle, float ReticleScale, bool canFire)
{
	//reticle
	UpdateWeaponReticleHUD_Vehicle(Reticle);
	UpdateWeaponReticleSize_Vehicle(ReticleScale);

	//weapon name
	UpdateWeaponNameHUD_Vehicle(WeaponName);

	//weapon status
	UpdateWeaponStatusHUD_Vehicle(canFire);

}

void UHUDSubsystem::UpdateWeaponReticleHUD_Vehicle(UTexture2D* ImageBrush)
{
	if (CurrentVehicleHUD && CurrentVehicleHUD->VehicleWeaponReticle)
	{
		CurrentVehicleHUD->VehicleWeaponReticle->UpdateReticleImage(ImageBrush);
	}
}

void UHUDSubsystem::UpdateWeaponReticleSize_Vehicle(float NewScale)
{
	//for zoom optic and initial size
	if (CurrentVehicleHUD && CurrentVehicleHUD->VehicleWeaponReticle)
	{
		CurrentVehicleHUD->VehicleWeaponReticle->UpdateReticleScale(NewScale);
	}
}

void UHUDSubsystem::UpdateRangefinderHUD_Vehicle(float NewRange)
{
	if (CurrentVehicleHUD && CurrentVehicleHUD->Rangefinder)
	{
		CurrentVehicleHUD->Rangefinder->UpdateRangefinder(NewRange);
	}
}

void UHUDSubsystem::UpdateWeaponNameHUD_Vehicle(FText WeaponDisplayName)
{
	if (CurrentVehicleHUD && CurrentVehicleHUD->VehicleWeaponStatus)
	{
		CurrentVehicleHUD->VehicleWeaponStatus->UpdateWeaponName(WeaponDisplayName);
	}
}

void UHUDSubsystem::UpdateWeaponStatusHUD_Vehicle(bool canFire)
{
	if (CurrentVehicleHUD && CurrentVehicleHUD->VehicleWeaponStatus)
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
		CurrentVehicleHUD->VehicleWeaponStatus->UpdateWeaponStatus(WeaponStatus);
	}
}

void UHUDSubsystem::UpdateCompassHUD_Vehicle(float Yaw)
{
	//should be bound to BOTH vehicle steer and control turret events as thats what determines a players compass in a vehicle
	//get seat state, active camera component rotation
	if (CurrentVehicleHUD && CurrentVehicleHUD->Compass)
	{
		CurrentVehicleHUD->Compass->UpdateCompassPosition(Yaw);
	}
}

void UHUDSubsystem::UpdateTurretLinesHUD_Vehicle()
{
	APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	ACharacter_Base* Character = (PC) ? Cast<ACharacter_Base>(PC->GetPawn()) : nullptr;
	AVehicle_Base* Vehicle = Character->GetCurrentVehicle();
	if (CurrentVehicleHUD && CurrentVehicleHUD->TurretLines)
	{
		for (int32 i = 0; i < Vehicle->VehicleWeaponLogicComponent->TurretStates.Num(); i++)
		{
			float RelativeYaw = Vehicle->VehicleWeaponLogicComponent->GetTurretWorldYaw(i);
			CurrentVehicleHUD->TurretLines->UpdateTurretLinePosition(i, RelativeYaw);
		}
	}
}

void UHUDSubsystem::UpdateTurretElevationHUD_Vehicle(float MinPitch, float MaxPitch, float CurrentPitch)
{
	if (CurrentVehicleHUD && CurrentVehicleHUD->TurretElvGauge)
	{
		CurrentVehicleHUD->TurretElvGauge->UpdateElevationGauge(CurrentPitch, MinPitch, MaxPitch);
	}
	if (CurrentVehicleHUD && CurrentVehicleHUD->TurretPitchMeter)
	{
		CurrentVehicleHUD->TurretPitchMeter->UpdatePitchMeter(CurrentPitch);
	}
}

void UHUDSubsystem::HandleTurretRotationUpdate(float Yaw)
{
	//should be bound ONLY to turret rotation

	if (CurrentVehicleHUD && CurrentVehicleHUD->Compass)
	{
		UpdateCompassHUD_Vehicle(Yaw);
	}
	// --- EVERYBODY UPDATES TURRET LINES HUD TO REFLECT CHANGE ---
	UpdateTurretLinesHUD_Vehicle();
}

void UHUDSubsystem::HandleTurretPitchUpdate(float MinPitch, float MaxPitch, float CurrentPitch)
{
	UpdateTurretElevationHUD_Vehicle(MinPitch, MaxPitch, CurrentPitch);
}

void UHUDSubsystem::RemoveWidget(UUW_HUD_LockOnIndicator_Base* UserWidget)
{
	UserWidget->RemoveFromParent();
	LockOnIndicator = nullptr; //HARDCODED FOR NOW, CHANGE THIS
}
