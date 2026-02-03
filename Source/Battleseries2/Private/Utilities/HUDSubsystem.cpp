#include "Utilities/HUDSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Core/UI/GameplayHUDs/UW_HUD_Status_Base.h"
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
	ACharacter_Base* Character = (PC) ? Cast<ACharacter_Base>(PC->GetPawn()) : nullptr;
	StatusHUD = CreateWidget<UUW_HUD_Status_Base>(PC, HUDClass);
	StatusHUD->AddToViewport();
}

void UHUDSubsystem::SpawnVehicleSeatHUD(TSubclassOf<UUserWidget> HUDClass)
{
	//called on enter seat
	APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	ACharacter_Base* Character = (PC) ? Cast<ACharacter_Base>(PC->GetPawn()) : nullptr;
	AVehicle_Base* Vehicle = Character->CharacterState.CharacterVehicleState.CurrentVehicle;

	CurrentVehicleHUD = CreateWidget<UUW_HUD_Vehicle_Base>(PC, HUDClass);
	CurrentVehicleHUD->AddToViewport();

	//do everything at least once maybe?
}

void UHUDSubsystem::SetupVehicleDriverHUD()
{
	//called on enter of driver seat, hud should be from created via the seat we just entered (seatdata.seathud)
	APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	ACharacter_Base* Character = (PC) ? Cast<ACharacter_Base>(PC->GetPawn()) : nullptr;
	if (!CurrentVehicleHUD)
	{
		return;
	}
	AVehicle_Base* Vehicle = Character->CharacterState.CharacterVehicleState.CurrentVehicle;
	Vehicle->OnVehicleSpeedUpdate.AddDynamic(this, &UHUDSubsystem::UpdateSpeedHUD_Vehicle);

	//call once to update/sync on start
	UpdateSpeedHUD_Vehicle();

	//aircraft hud elements?
}

void UHUDSubsystem::SetupVehicleGunnerHUD()
{
	//called on enter of gunner seat, hud should be from created via the seat we just entered (seatdata.seathud)
	APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	ACharacter_Base* Character = (PC) ? Cast<ACharacter_Base>(PC->GetPawn()) : nullptr;
	if (!CurrentVehicleHUD)
	{
		return;
	}
	AVehicle_Base* Vehicle = Character->CharacterState.CharacterVehicleState.CurrentVehicle;
	int32& CSI = Character->CharacterState.CharacterVehicleState.CSI;
	Vehicle->OnVehicleYawUpdate.AddUniqueDynamic(this, &UHUDSubsystem::UpdateCompassHUD_Vehicle);	
	Vehicle->VehicleWeaponLogicComponent->OnTurretRotated.AddUniqueDynamic(this, &UHUDSubsystem::HandleTurretRotationUpdate);
	Vehicle->VehicleWeaponLogicComponent->OnTurretPitched.AddUniqueDynamic(this, &UHUDSubsystem::HandleTurretPitchUpdate);
	Vehicle->VehicleWeaponLogicComponent->OnVehicleWeaponFired.AddUniqueDynamic(this, &UHUDSubsystem::UpdateStatusHUD_CAMCount_Vehicle);
	Vehicle->VehicleWeaponLogicComponent->OnVehicleWeaponSwapped.AddUniqueDynamic(this, &UHUDSubsystem::UpdateEquippedWeaponHUD_Vehicle);
	//reload
	//shoot

	//call once to update/sync on start
	UpdateStatusHUD_CAMCount_Vehicle(CSI);
	UpdateEquippedWeaponHUD_Vehicle(CSI);		//reticle, name, status
	UpdateCompassHUD_Vehicle(CSI);
	HandleTurretRotationUpdate(CSI);
	HandleTurretPitchUpdate(CSI);
	//optic
	//countermeasure
}

void UHUDSubsystem::UpdateStatusHUD_CAMCount_Vehicle(int32 SeatIndex)
{
	APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	ACharacter_Base* Character = (PC) ? Cast<ACharacter_Base>(PC->GetPawn()) : nullptr;
	AVehicle_Base* Vehicle = Character->CharacterState.CharacterVehicleState.CurrentVehicle;
	if (SeatIndex != Character->CharacterState.CharacterVehicleState.CSI)
	{
		return;
	}
	int32& CWI = Vehicle->VehicleWeaponLogicComponent->VehicleWeaponSystem.Find(SeatIndex)->VehicleWeaponSystemState.EquippedWeaponState.CurrentWeaponIndex;
	int32& CAM = Vehicle->VehicleWeaponLogicComponent->VehicleWeaponSystem.Find(SeatIndex)->Weapons[CWI].VehicleWeaponState.BaseWeaponRuntimeData.WeaponState.CurrentAmmoinMag;
	
	StatusHUD->UpdateCAMCount(CAM);
}

void UHUDSubsystem::UpdateSpeedHUD_Vehicle()
{
	APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	ACharacter_Base* Character = (PC) ? Cast<ACharacter_Base>(PC->GetPawn()) : nullptr;

	if (CurrentVehicleHUD->Speedometer)
	{
		float RawSpeed = Character->CharacterState.CharacterVehicleState.CurrentVehicle->GetVelocity().Size();
		//use chaos speed?
		float DisplaySpeed = RawSpeed * 0.036f;		// Unreal Units to KPH
		CurrentVehicleHUD->Speedometer->UpdateSpeedometer(DisplaySpeed);
	}
}

void UHUDSubsystem::UpdateEquippedWeaponHUD_Vehicle(int32 SeatIndex)
{
	//reticle, name, status (ready, wait)
	APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	ACharacter_Base* Character = (PC) ? Cast<ACharacter_Base>(PC->GetPawn()) : nullptr;
	AVehicle_Base* Vehicle = Character->CharacterState.CharacterVehicleState.CurrentVehicle;
	if (SeatIndex != Character->CharacterState.CharacterVehicleState.CSI)
	{
		return;
	}

	//reticle
	int32& CWI = Vehicle->VehicleWeaponLogicComponent->VehicleWeaponSystem.Find(SeatIndex)->VehicleWeaponSystemState.EquippedWeaponState.CurrentWeaponIndex;
	UpdateWeaponReticleHUD_Vehicle(Vehicle->VehicleWeaponLogicComponent->VehicleWeaponSystem.Find(SeatIndex)->Weapons[CWI].VehicleWeaponInstanceData.WeaponReticle);
	UpdateWeaponReticleSize_Vehicle(Vehicle->VehicleWeaponLogicComponent->VehicleWeaponSystem.Find(SeatIndex)->Weapons[CWI].VehicleWeaponInstanceData.ReticleScale);

	const FBaseWeaponData& StaticWeaponData = Vehicle->VehicleWeaponLogicComponent->GetBaseWeaponDataInSlot(SeatIndex, CWI);

	//weapon name
	UpdateWeaponNameHUD_Vehicle(StaticWeaponData.WeaponClassification.WeaponDisplayNameAbrev);
	FName WeaponID = Vehicle->VehicleWeaponLogicComponent->VehicleWeaponSystem.Find(SeatIndex)->Weapons[CWI].VehicleWeaponState.BaseWeaponRuntimeData.WeaponID;
	UE_LOG(LogTemp, Warning, TEXT("[HUDSubsystem::UpdateEquippedWeaponHUD_Vehicle] WeaponID = %s"), *WeaponID.ToString());
	UE_LOG(LogTemp, Warning, TEXT("[HUDSubsystem::UpdateEquippedWeaponHUD_Vehicle] WeaponDisplayNameAbrev = %s"), *StaticWeaponData.WeaponClassification.WeaponDisplayNameAbrev.ToString());

	//weapon status
	FText WeaponStatus;
	if (Vehicle->VehicleWeaponLogicComponent->VehicleWeaponSystem.Find(SeatIndex)->Weapons[CWI].VehicleWeaponState.BaseWeaponRuntimeData.WeaponState.isReloading)
	{
		WeaponStatus = FText::FromString("WAIT");
	}
	else
	{
		WeaponStatus = FText::FromString("READY");
	}
	UpdateWeaponStatusHUD_Vehicle(WeaponStatus);

}

void UHUDSubsystem::UpdateWeaponReticleHUD_Vehicle(UTexture2D* ImageBrush)
{
	APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	ACharacter_Base* Character = (PC) ? Cast<ACharacter_Base>(PC->GetPawn()) : nullptr;
	int32& SI = Character->CharacterState.CharacterVehicleState.CSI;
	AVehicle_Base* Vehicle = Character->CharacterState.CharacterVehicleState.CurrentVehicle;
	if (CurrentVehicleHUD->VehicleWeaponReticle)
	{
		CurrentVehicleHUD->VehicleWeaponReticle->UpdateReticleImage(ImageBrush);
	}
}

void UHUDSubsystem::UpdateWeaponReticleSize_Vehicle(float NewScale)
{
	//for zoom optic and initial size
	APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	ACharacter_Base* Character = (PC) ? Cast<ACharacter_Base>(PC->GetPawn()) : nullptr;
	if (CurrentVehicleHUD->VehicleWeaponReticle)
	{
		CurrentVehicleHUD->VehicleWeaponReticle->UpdateReticleScale(NewScale);
	}
}

void UHUDSubsystem::UpdateRangefinderHUD_Vehicle(AVehicle_Base* ReportingVehicle, int32 SeatIndex, float NewRange)
{
	if (!CurrentVehicleHUD)
	{
		return;
	}
	APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	ACharacter_Base* Character = (PC) ? Cast<ACharacter_Base>(PC->GetPawn()) : nullptr;
	int32& SI = Character->CharacterState.CharacterVehicleState.CSI;
	AVehicle_Base* Vehicle = Character->CharacterState.CharacterVehicleState.CurrentVehicle;
	if (Vehicle == ReportingVehicle && SI == SeatIndex)
	{
		if (CurrentVehicleHUD->Rangefinder)
		{
			CurrentVehicleHUD->Rangefinder->UpdateRangefinder(NewRange);
		}
	}
}

void UHUDSubsystem::UpdateWeaponNameHUD_Vehicle(FText WeaponDisplayName)
{
	APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	ACharacter_Base* Character = (PC) ? Cast<ACharacter_Base>(PC->GetPawn()) : nullptr;
	int32& SI = Character->CharacterState.CharacterVehicleState.CSI;
	AVehicle_Base* Vehicle = Character->CharacterState.CharacterVehicleState.CurrentVehicle;
	if (CurrentVehicleHUD->VehicleWeaponStatus)
	{
		CurrentVehicleHUD->VehicleWeaponStatus->UpdateWeaponName(WeaponDisplayName);
	}
}

void UHUDSubsystem::UpdateWeaponStatusHUD_Vehicle(FText WeaponStatus)
{
	APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	ACharacter_Base* Character = (PC) ? Cast<ACharacter_Base>(PC->GetPawn()) : nullptr;
	if (CurrentVehicleHUD->VehicleWeaponStatus)
	{
		CurrentVehicleHUD->VehicleWeaponStatus->UpdateWeaponStatus(WeaponStatus);
	}
}

void UHUDSubsystem::UpdateCompassHUD_Vehicle(int32 SeatIndex)
{
	//should be bound to BOTH vehicle steer and control turret events as thats what determines a players compass in a vehicle
	APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	ACharacter_Base* Character = (PC) ? Cast<ACharacter_Base>(PC->GetPawn()) : nullptr;
	int32& SI = Character->CharacterState.CharacterVehicleState.CSI;
	AVehicle_Base* Vehicle = Character->CharacterState.CharacterVehicleState.CurrentVehicle;
	Vehicle->VehicleCurrentState.SeatStates[SI].ActiveCamera->GetComponentRotation().Yaw;

	if (SeatIndex != Character->CharacterState.CharacterVehicleState.CSI)
	{
		return;
	}
	if (CurrentVehicleHUD->Compass)
	{
		CurrentVehicleHUD->Compass->UpdateCompassPosition(Vehicle->VehicleCurrentState.SeatStates[SI].ActiveCamera->GetComponentRotation().Yaw);
	}
}

void UHUDSubsystem::UpdateTurretLinesHUD_Vehicle()
{
	APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	ACharacter_Base* Character = (PC) ? Cast<ACharacter_Base>(PC->GetPawn()) : nullptr;
	AVehicle_Base* Vehicle = Character->CharacterState.CharacterVehicleState.CurrentVehicle;
	if (CurrentVehicleHUD->TurretLines)
	{
		for (int32 i = 0; i < Vehicle->VehicleWeaponLogicComponent->TurretStates.Num(); i++)
		{
			float RelativeYaw = Vehicle->VehicleWeaponLogicComponent->GetTurretWorldYaw(i);
			CurrentVehicleHUD->TurretLines->UpdateTurretLinePosition(i, RelativeYaw);
		}
	}
}

void UHUDSubsystem::UpdateTurretElevationHUD_Vehicle(int32 SeatIndex)
{
	APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	ACharacter_Base* Character = (PC) ? Cast<ACharacter_Base>(PC->GetPawn()) : nullptr;
	AVehicle_Base* Vehicle = Character->CharacterState.CharacterVehicleState.CurrentVehicle;

	int32 CTI = Vehicle->GetVehicleData().Seats[SeatIndex].AvailableItems.ControlledTurretIndexes[0];
	float CurrentPitch = Vehicle->VehicleWeaponLogicComponent->TurretStates[CTI].CurrentTurretPitch;
	if (CurrentVehicleHUD->TurretElvGauge)
	{
		float MinPitch = Vehicle->GetVehicleData().Turrets[CTI].TurretPitch.TurretMinMax.GetLowerBoundValue();
		float MaxPitch = Vehicle->GetVehicleData().Turrets[CTI].TurretPitch.TurretMinMax.GetUpperBoundValue();
		CurrentVehicleHUD->TurretElvGauge->UpdateElevationGauge(CurrentPitch, MinPitch, MaxPitch);
	}
	if (CurrentVehicleHUD->TurretPitchMeter)
	{
		CurrentVehicleHUD->TurretPitchMeter->UpdatePitchMeter(CurrentPitch);
	}
}

void UHUDSubsystem::HandleTurretRotationUpdate(int32 SeatIndex)
{
	//should be bound ONLY to turret rotation
	APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	ACharacter_Base* Character = (PC) ? Cast<ACharacter_Base>(PC->GetPawn()) : nullptr;
	if (SeatIndex == Character->CharacterState.CharacterVehicleState.CSI && CurrentVehicleHUD->Compass)
	{
		UpdateCompassHUD_Vehicle(SeatIndex);
	}
	// --- EVERYBODY UPDATES TURRET LINES HUD TO REFLECT CHANGE ---
	UpdateTurretLinesHUD_Vehicle();
}

void UHUDSubsystem::HandleTurretPitchUpdate(int32 SeatIndex)
{
	UpdateTurretElevationHUD_Vehicle(SeatIndex);
}
