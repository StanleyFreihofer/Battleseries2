// coding in UE almost makes me wish for a nuclear winter


#include "Core/Vehicles/CharacterVehicleManager.h"
#include "Data/Vehicles/VehicleTypes.h"
#include "Data/Characters/CharacterDefaults.h"
#include "Character_Base.h"
#include "Vehicle_Base.h"
#include "Core/Weapons/LoadoutManager.h"
#include "Core/Weapons/VehicleWeaponLogicComponent.h"
#include "Core/UI/VehicleHUDs/UW_HUD_Vehicle_Base.h"
#include "Utilities/HUDSubsystem.h"
#include "Utilities/DataManagerSubsystem.h"
#include "Utilities/BS2FunctionLibrary.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values for this component's properties
UCharacterVehicleManager::UCharacterVehicleManager()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}



// Called when the game starts
void UCharacterVehicleManager::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UCharacterVehicleManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UCharacterVehicleManager::ManageinVehicleStatus(AVehicle_Base* Vehicle, bool In_Vehicle)
{
	CharacterVehicleState.inVehicle = In_Vehicle;
	CharacterVehicleState.CurrentVehicle = Vehicle;
	if (CharacterVehicleState.inVehicle)
	{
		CharacterEnterVehicle();
	}
	else
	{
		CharacterExitVehicle();
	}
}

void UCharacterVehicleManager::UpdateSeatList(TArray<ACharacter_Base*> Characters)
{
	TArray<FSeatState> Seats = CharacterVehicleState.CurrentVehicle->VehicleCurrentState.SeatStates;
	for (int32 SI = 0; SI < Seats.Num(); SI++)
	{
		for (ACharacter_Base* Character : Characters)
		{
			if (CharacterVehicleState.CSI == SI)
			{
				//do something UI here (show seat as occupied, that characters name, etc)
				continue;
			}
		}
	}
}

void UCharacterVehicleManager::CharacterEnterVehicle()
{
	//if Vehicle RC Data does actually become a thing, certain things here need to be blocked based on that new property's value
	if (!GetCurrentVehicle()->VehicleData->bCanRemoteControl)
	{
		PhysicallyEnterVehicle();
	}

	GetOwnerCharacter()->ManageIMC(UBS2FunctionLibrary::GetDataSubsystem(this)->GetCharacterDefaults()->DefaultGameplayIMC.Get(), nullptr, -1);
	if (GetOwnerCharacter()->IsLocallyControlled())
	{
		EnterVehicle_LocalPlayer();
	}
}

void UCharacterVehicleManager::EnterVehicle_LocalPlayer()
{
	GetCurrentVehicle()->VehicleHealthComponent->OnVehicleHealthChanged.AddDynamic(this, &UCharacterVehicleManager::OnVehicleHealthChanged);
	UBS2FunctionLibrary::GetHUDSubsystem(this)->UpdateStatusHUD_VehicleHealth(GetCurrentVehicle()->GetVehicleHealth());
	UBS2FunctionLibrary::GetHUDSubsystem(this)->UpdateStatusHUD_VehicleStatusVisibility(false);
}

void UCharacterVehicleManager::PhysicallyEnterVehicle()
{
	GetOwnerCharacter()->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Ignore);
	GetOwnerCharacter()->GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Vehicle, ECollisionResponse::ECR_Ignore);
	GetOwnerCharacter()->FPArms->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Ignore);
	GetOwnerCharacter()->FPLegs->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Ignore);
	for (int32 i = 0; i < GetOwnerCharacter()->LoadoutManager->GetNumWeapons(); i++)
	{
		GetOwnerCharacter()->LoadoutManager->UpdateWeaponCollision(ECC_Vehicle, ECR_Ignore, i);
	}
	for (int32 G = 0; G < GetOwnerCharacter()->LoadoutManager->GetNumGadgets(); G++)
	{
		GetOwnerCharacter()->LoadoutManager->UpdateGadgetCollision(ECC_Vehicle, ECR_Ignore, G);
	}
	GetOwnerCharacter()->GetCharacterMovement()->SetMovementMode(MOVE_None);
	GetOwnerCharacter()->AttachToActor(GetCurrentVehicle(), FAttachmentTransformRules::KeepRelativeTransform);
		
	GetOwnerCharacter()->FPArmsSpringArm->bUsePawnControlRotation = false;
	GetOwnerCharacter()->FPArmsSpringArm->bInheritRoll = true;
	GetOwnerCharacter()->FPArmsSpringArm->bEnableCameraLag = false;
	GetOwnerCharacter()->FPCamera->bUsePawnControlRotation = false;
	GetOwnerCharacter()->FPCamera->SetRelativeRotation(FRotator());
	GetOwnerCharacter()->bUseControllerRotationYaw = false;
}

void UCharacterVehicleManager::CharacterExitVehicle()
{
	if (GetCurrentVehicle())
	{
		//CharacterExitSeat(GetCurrentVehicle()->VehicleData->Seats[GetCSI()].DefaultCharacterContext);
		GetCurrentVehicle()->DropSeat(GetOwnerCharacter(), GetCSI());
		
		if (!GetCurrentVehicle()->VehicleData->bCanRemoteControl)
		{
			PhysicallyExitVehicle();
		}

		GetOwnerCharacter()->UpdateViewTarget(GetOwnerCharacter(), GetOwnerCharacter()->FPCamera);

		GetOwnerCharacter()->ManageIMC(nullptr, UBS2FunctionLibrary::GetDataSubsystem(this)->GetCharacterDefaults()->DefaultGameplayIMC.Get(), 1);
	
		if (GetOwnerCharacter()->LoadoutManager->GetIsCurrentSlotActuallyWeapon() && GetOwnerCharacter()->IsLocallyControlled())
		{
			ExitVehicle_LocalPlayer();
		}
		
		CharacterVehicleState = FCharacterVehicleState();
	}
}

void UCharacterVehicleManager::ExitVehicle_LocalPlayer()
{
	GetCurrentVehicle()->VehicleHealthComponent->OnVehicleHealthChanged.RemoveDynamic(this, &UCharacterVehicleManager::OnVehicleHealthChanged);
	FWeaponState& CurrentWeapon = *GetOwnerCharacter()->LoadoutManager->GetCurrentWeaponBaseState();
	UBS2FunctionLibrary::GetHUDSubsystem(this)->UpdateStatusHUD_CAMCount(CurrentWeapon.CurrentAmmoinMag);
	UBS2FunctionLibrary::GetHUDSubsystem(this)->UpdateStatusHUD_CRACount(CurrentWeapon.CurrentReserveAmmo);
	UBS2FunctionLibrary::GetHUDSubsystem(this)->UpdateStatusHUD_VehicleStatusVisibility(true);
}

void UCharacterVehicleManager::PhysicallyExitVehicle()
{
	GetOwnerCharacter()->DetachFromActor(FDetachmentTransformRules::KeepRelativeTransform);

	FVector ExitLocation = CalculateSafeExitLocation(GetCurrentVehicle());
	GetOwnerCharacter()->SetActorLocation(ExitLocation);

	GetOwnerCharacter()->HandleUpdateStance(ECharacterStance::Standing);
	GetOwnerCharacter()->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Block);
	GetOwnerCharacter()->GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Vehicle, ECollisionResponse::ECR_Block);
	GetOwnerCharacter()->GetCharacterMovement()->SetMovementMode(MOVE_Walking);		//make this more dynamic (are we falling out of ejecting from a jet for example)

	GetOwnerCharacter()->FPCamera->bUsePawnControlRotation = true;
	GetOwnerCharacter()->bUseControllerRotationYaw = true;
	GetOwnerCharacter()->FPArmsSpringArm->bUsePawnControlRotation = true;
	GetOwnerCharacter()->FPArmsSpringArm->bInheritRoll = false;
	GetOwnerCharacter()->FPArmsSpringArm->bEnableCameraLag = true;
}

void UCharacterVehicleManager::CharacterEnterSeat(const FCharacterSeatContext& SeatContext)
{
	//if Vehicle RC Data does actually become a thing, Character transform should not be set (block switch behind if)
	if (!GetCurrentVehicle()->VehicleData->bCanRemoteControl)
	{
		switch (GetCurrentVehicle()->GetVehicleData().Seats[GetCSI()].SeatRole)
		{
			case E_SeatRole::Driver:
			case E_SeatRole::Passenger:
				GetOwnerCharacter()->SetActorRelativeTransform(SeatContext.SeatTransform);
				break;
			case E_SeatRole::DriverGunner:
			case E_SeatRole::Gunner:
				HandleEnterSeat_Gunner(SeatContext);
				break;
		}

		GetOwnerCharacter()->HandleUpdateStance(SeatContext.SeatStance);
		GetOwnerCharacter()->UpdateCharacterMeshVisibility(SeatContext.bIsCharacterVisible);
	}
	
	GetOwnerCharacter()->ManageIMC(nullptr, SeatContext.InputMappingContext, 1);

	if (SeatContext.SeatHMD)
	{
		UpdateVehicleHUD(SeatContext.SeatHMD);
	}

	UpdateUI_EnterSeat();
}

void UCharacterVehicleManager::HandleEnterSeat_Gunner(const FCharacterSeatContext& SeatContext)
{
	TObjectPtr<UVehicleWeaponLogicComponent> VWLC = GetCurrentVehicle()->VehicleWeaponLogicComponent;
	const FVehicleWeaponInstanceData& VWID = VWLC->GetVWID(GetCSI(), VWLC->GetCWIForSeat(GetCSI()), VWLC->GetEquippedWeaponIDInSeat(GetCSI()));
	if (VWID.AttachmentInstanceData.bAttachCharacter)
	{
		TWeakObjectPtr<USkeletalMeshComponent> WeaponMesh = VWLC->VehicleWeaponSystem.Find(GetCSI())->VehicleWeaponSystemState.WeaponSystemMesh;
		FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, false);
		GetOwnerCharacter()->GetRootComponent()->AttachToComponent(WeaponMesh.Get(), AttachmentRules, FName("Test"));
		GetOwnerCharacter()->SetActorRelativeTransform(VWID.AttachmentInstanceData.CharacterTransform);
	}
	else
	{
		GetOwnerCharacter()->SetActorRelativeTransform(SeatContext.SeatTransform);
	}
}

void UCharacterVehicleManager::CharacterExitSeat(const FCharacterSeatContext& SeatContext)
{
	if (!GetCurrentVehicle()->VehicleData->bCanRemoteControl)
	{
		if (GetOwnerCharacter()->GetAttachParentActor()->GetRootComponent() != GetOwnerCharacter()->GetRootComponent()->GetAttachParent())
		{
			GetOwnerCharacter()->GetRootComponent()->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
			GetOwnerCharacter()->AttachToActor(GetCurrentVehicle(), FAttachmentTransformRules::KeepRelativeTransform);
		}
	}

	UpdateVehicleHUD(nullptr);
	GetOwnerCharacter()->ManageIMC(SeatContext.InputMappingContext, nullptr, 0);
}

FVector UCharacterVehicleManager::CalculateSafeExitLocation(AActor* Vehicle)
{
	// Define exit points relative to the vehicle (Right, Left, Back)
	TArray<FVector> ExitOffsets;
	ExitOffsets.Add(FVector(0, 250, 50));   // Right
	ExitOffsets.Add(FVector(0, -250, 50));  // Left
	ExitOffsets.Add(FVector(-300, 0, 50));  // Back

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Vehicle);
	Params.AddIgnoredActor(GetOwner());

	// Use the actual capsule shape for the sweep
	FCollisionShape GraduationCapsule = GetOwnerCharacter()->GetCapsuleComponent()->GetCollisionShape();
	FVector VehicleLoc = Vehicle->GetActorLocation();

	for (const FVector& Offset : ExitOffsets)
	{
		FVector TargetLocation = Vehicle->GetActorTransform().TransformPosition(Offset);

		// Ensure the exit point is on the ground (Project down)
		FHitResult GroundHit;
		FVector GroundCheckStart = TargetLocation + FVector(0, 0, 100);
		FVector GroundCheckEnd = TargetLocation - FVector(0, 0, 500);

		if (GetWorld()->LineTraceSingleByChannel(GroundHit, GroundCheckStart, GroundCheckEnd, ECC_WorldStatic, Params))
		{
			TargetLocation = GroundHit.ImpactPoint + FVector(0, 0, GraduationCapsule.GetCapsuleHalfHeight());
		}

		// Final check: Does the capsule actually fit here without overlapping?
		if (!GetWorld()->OverlapBlockingTestByChannel(TargetLocation, FQuat::Identity, ECC_Pawn, GraduationCapsule, Params))
		{
			return TargetLocation;
		}
	}

	// Fallback: If all else fails, try a point slightly further away or above
	return Vehicle->GetActorLocation() + (Vehicle->GetActorUpVector() * 250.0f);
}

void UCharacterVehicleManager::UpdateSeatIndexes(int32 NewLSI, int32 NewCSI, int32 NewNSI)
{
	CharacterVehicleState.LSI = NewLSI;
	CharacterVehicleState.CSI = NewCSI;
	CharacterVehicleState.NSI = NewNSI;
}

void UCharacterVehicleManager::UpdateVehicleHUD(TSubclassOf<UUserWidget> HUDClass)
{
	if (TObjectPtr<UHUDSubsystem> HUDSub = UBS2FunctionLibrary::GetHUDSubsystem(this))
	{
		if (!HUDSub->CurrentVehicleHMD && HUDClass)
		{
			HUDSub->SpawnVehicleSeatHUD(HUDClass);
		}
		else if (!HUDClass && HUDSub->CurrentVehicleHMD)
		{
			HUDSub->CurrentVehicleHMD->RemoveFromParent();
			HUDSub->CurrentVehicleHMD = nullptr;
		}
	}
}

void UCharacterVehicleManager::UpdateUI_EnterSeat()
{
	//sync vehicle states for hud
	switch (GetCurrentVehicle()->VehicleData->Seats[GetCSI()].SeatRole)
	{
		case E_SeatRole::Driver:
			UBS2FunctionLibrary::GetHUDSubsystem(this)->UpdateSpeedHUD_Vehicle(GetCurrentVehicle()->GetCurrentSpeed());
			break;
		case E_SeatRole::Gunner:
			UpdateUI_EnterSeat_Turrets();
			break;
		case E_SeatRole::DriverGunner:
			//HUD
			UBS2FunctionLibrary::GetHUDSubsystem(this)->UpdateSpeedHUD_Vehicle(GetCurrentVehicle()->GetCurrentSpeed());
			UpdateUI_EnterSeat_Turrets();
			break;
	}
}

void UCharacterVehicleManager::UpdateUI_EnterSeat_Turrets()
{
	//turrets/heading
	if (UCameraComponent* ActiveCam = GetCurrentVehicle()->GetRemoteActiveCam(GetCSI()))
	{
		UBS2FunctionLibrary::GetHUDSubsystem(this)->UpdateCompassHUD_Vehicle(ActiveCam->GetComponentRotation().Yaw);
		UBS2FunctionLibrary::GetHUDSubsystem(this)->HandleTurretRotationUpdate(ActiveCam->GetComponentRotation().Yaw);
	}
	if (GetCurrentVehicle()->VehicleData->Seats[GetCSI()].AvailableItems.ControlledTurretIndexes.Num())
	{
		const int32& CTI = GetCurrentVehicle()->GetControlledTurret(GetCSI());
				
		float MinPitch, MaxPitch, CurrentPitch;
		GetCurrentVehicle()->GetTurretPitchRange(CTI, MinPitch, MaxPitch, CurrentPitch);

		UBS2FunctionLibrary::GetHUDSubsystem(this)->HandleTurretPitchUpdate(MinPitch, MaxPitch, CurrentPitch);
	}
}

void UCharacterVehicleManager::OnVehicleHealthChanged()
{
	if (GetOwnerCharacter()->IsLocallyControlled())
	{
		UBS2FunctionLibrary::GetHUDSubsystem(this)->UpdateStatusHUD_VehicleHealth(GetCurrentVehicle()->GetVehicleHealth());
	}
}

void UCharacterVehicleManager::UpdateRangefinder_WindowedVehicle()
{
	//free looking? (make sure its correctly managed this time)
	if (!GetInVehicle()|| !GetCurrentVehicle())
	{
		return;
	}

	const FSeatData& OccupiedSeatData = GetCurrentVehicle()->VehicleData->Seats[GetCSI()];
	if (OccupiedSeatData.ViewMethod != E_ViewMethod::Windowed)
	{
		return;
	}

	TWeakObjectPtr<UVehicleWeaponLogicComponent> VWLC = GetCurrentVehicle()->VehicleWeaponLogicComponent;
	FVehicleWeapon_Runtime& CurrentWeapon = VWLC->GetEquippedWeaponInSeat(GetCSI());
	TArray<AActor*> IgnoreActors = {GetCurrentVehicle(), GetOwner() };
	FTransform TraceTransform;
	FVector PlayerEyePos;

	switch (CurrentWeapon.VehicleWeaponInstanceData.WindowedAimAnchor)
	{
		case EWindowedAimAnchor::FreeAim:
			TraceTransform = GetOwnerCharacter()->FPCamera->GetComponentTransform();
			PlayerEyePos = TraceTransform.GetLocation();
			break;
		case EWindowedAimAnchor::FixedHead:
		{
			FVector StartLocation = GetOwnerCharacter()->FPArms->GetSocketLocation(FName("FixedCamera"));
			TraceTransform = FTransform(GetOwner()->GetActorQuat(), StartLocation);
			PlayerEyePos = StartLocation;
			break;
		}
		case EWindowedAimAnchor::FixedPoint:
		{
			FString SocketString = FString::Printf(TEXT("SC_%02d"), GetCSI());
			FName SocketName = FName(*SocketString);
			FVector StartLocation = GetCurrentVehicle()->VehicleMeshComponent->GetSocketLocation(SocketName);
			TraceTransform = FTransform(GetCurrentVehicle()->GetActorQuat(), StartLocation);
			PlayerEyePos = StartLocation;
			break;
		}
		case EWindowedAimAnchor::Hull:
		{
			FVector HullStart = GetCurrentVehicle()->GetActorLocation() + (GetCurrentVehicle()->GetActorUpVector() * 100.0f);
			TraceTransform = FTransform(GetCurrentVehicle()->GetActorQuat(), HullStart);
			PlayerEyePos = HullStart;
			break;
		}
	}

	VWLC->UpdateSeatRangefinder(GetCSI(), TraceTransform, IgnoreActors);

	if (!GetOwnerCharacter()->IsLocallyControlled())
	{
		return;
	}

	UWidgetComponent* SeatHUDComp = GetCurrentVehicle()->VehicleCurrentState.SeatStates[GetCSI()].SeatHUDComponent;
	if (!SeatHUDComp)
	{
		return;
	}

	// Retrieve Hit and Component Data
	auto* WeaponSystem = VWLC->VehicleWeaponSystem.Find(GetCSI());

	FHitResult& HitResult = WeaponSystem->VehicleWeaponSystemState.EquippedWeaponState.RaycastData.RangefinderData;
	TObjectPtr<UStaticMeshComponent> Quad = WeaponSystem->VehicleWeaponSystemState.ReticleQuad.Get();

	FString SocketString = FString::Printf(TEXT("SC_%02d"), GetCSI());
	FName SocketName = FName(*SocketString);
	FVector StartLocation = GetCurrentVehicle()->VehicleMeshComponent->GetSocketLocation(SocketName);

	/**
	if (Quad)
	{
		FVector EyePos = StartLocation;	//FPCamera->GetComponentLocation();
		FVector TargetPos = HitResult.bBlockingHit ? HitResult.ImpactPoint : TraceTransform.GetLocation() + (TraceTransform.GetUnitAxis(EAxis::X) * 100000.0f);

		// Calculate where the eye-to-target line hits the HUD glass plane
		FVector IntersectionPoint = FMath::LinePlaneIntersection(EyePos, TargetPos, SeatHUDComp->GetComponentLocation(), SeatHUDComp->GetForwardVector());

		Quad->SetWorldLocation(IntersectionPoint);
	}
	**/
	FPlane HUDPlane = FPlane(SeatHUDComp->GetComponentLocation(), SeatHUDComp->GetForwardVector());
	if (Quad)
	{
		// 3. FIX PARALLAX: Determine what the eye point actually is for the player
		// If FreeAim, the player is look-controlling the camera. For others, they are looking through the fixed seat glass.


		// 4. FIX CONVERGENCE POINT: Establish exactly where the physical system hits
		FVector TargetImpactPos;
		if (HitResult.bBlockingHit)
		{
			TargetImpactPos = HitResult.ImpactPoint;
		}
		else
		{
			// Fallback if looking at the open sky: project out 1000 meters along the directional anchor axis
			TargetImpactPos = TraceTransform.GetLocation() + (TraceTransform.GetUnitAxis(EAxis::X) * 100000.0f);
		}

		// 5. PROJECT LINE UNTO HUD GLASS: Raycast from the actual player eye to the true impact point
		FVector IntersectionPoint;
		bool bIntersects = FMath::SegmentPlaneIntersection(
			PlayerEyePos,
			TargetImpactPos,
			HUDPlane,
			IntersectionPoint
		);

		if (bIntersects)
		{
			Quad->SetWorldLocation(IntersectionPoint);

			// Optional Quality of Life: Orient the quad face flat against the HUD glass plane
			//Quad->SetWorldRotation(SeatHUDComp->GetComponentRotation());

			// Ensure it stays visible when tracking cleanly
			if (!Quad->IsVisible())
			{
				Quad->SetVisibility(true);
			}
		}
		else
		{
			// If the trajectory convergence point falls entirely outside the viewport plane bounds, hide it
			Quad->SetVisibility(false);
		}
	}
}

void UCharacterVehicleManager::HandleViewMethod(const FSeatData& SeatData)
{
	//move to character?
	if (SeatData.SeatRole != E_SeatRole::DriverGunner && SeatData.SeatRole != E_SeatRole::Gunner)
	{
		HandleViewMethod_Default(SeatData);
		return;
	}

	int32 SeatIndex = GetCSI();

	GetCurrentVehicle()->SyncActiveCameraForSeat(SeatIndex);
	
	UCameraComponent* WeaponCam = GetCurrentVehicle()->GetSeatWeaponCam(SeatIndex);
	if (!WeaponCam)
	{
		HandleViewMethod_Default(SeatData);
		return;
	}
	
	TWeakObjectPtr<AActor> ViewTarget = GetCurrentVehicle()->VehicleWeaponLogicComponent->GetCurrentViewTargetAtSeatIndex(SeatIndex);
	GetOwnerCharacter()->UpdateViewTarget(ViewTarget, WeaponCam);
}

void UCharacterVehicleManager::HandleViewMethod_Default(const FSeatData& SeatData)
{
	//move to character?
	switch (SeatData.ViewMethod)
	{
		case E_ViewMethod::Windowed:
			GetOwnerCharacter()->UpdateViewTarget(GetOwnerCharacter(), GetOwnerCharacter()->FPCamera);
			break;
		case E_ViewMethod::Remote:
		{
			int32 SeatIndex = GetOwnerCharacter()->VehicleManager->GetCSI();
			GetCurrentVehicle()->SyncActiveCameraForSeat(SeatIndex);
			GetOwnerCharacter()->UpdateViewTarget(GetCurrentVehicle(), GetCurrentVehicle()->VehicleCurrentState.SeatStates[SeatIndex].DefaultCamera);
			break;
		}
	}
}

ACharacter_Base* UCharacterVehicleManager::GetOwnerCharacter()
{
	return Cast<ACharacter_Base>(GetOwner());
}

AVehicle_Base* UCharacterVehicleManager::GetCurrentVehicle()
{
	if (CharacterVehicleState.CurrentVehicle)
	{
		return CharacterVehicleState.CurrentVehicle;
	}
	else
	{
		return nullptr;
	}
}

int32& UCharacterVehicleManager::GetCSI()
{
	return CharacterVehicleState.CSI;
}

bool& UCharacterVehicleManager::GetInVehicle()
{
	return CharacterVehicleState.inVehicle;
}


