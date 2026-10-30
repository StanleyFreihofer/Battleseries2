// coding in UE almost makes me wish for a nuclear winter


#include "Core/Vehicles/CharacterVehicleManager.h"
#include "Data/Vehicles/VehicleTypes.h"
#include "Data/Characters/CharacterDefaults.h"
#include "Character_Base.h"
#include "Vehicle_Base.h"
#include "Core/Weapons/LoadoutManager.h"
#include "Core/Weapons/VehicleLoadoutManager.h"
#include "Core/UI/VehicleHUDs/UW_HUD_Vehicle_Base.h"
#include "Utilities/HUDSubsystem.h"
#include "Utilities/DataManagerSubsystem.h"
#include "Utilities/BS2FunctionLibrary.h"
#include "Animation/AnimInstance.h"
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
	CharacterVehicleState.CurrentVehicle = Vehicle;
	if (CharacterVehicleState.CurrentVehicle)
	{
		CharacterEnterVehicle();
	}
	else
	{
		CharacterExitVehicle();
	}
}

#pragma region EnterExitVehicle

#pragma region EnterVehicle

void UCharacterVehicleManager::AttemptEnterVehicle(AVehicle_Base* Vehicle)
{
	if (!Vehicle)	{ return; }
	if (Vehicle->GetIsVehicleDestroyed())	{return; }
	int32 SI = Vehicle->FindNextEmptySeat(GetCSI());
	if (SI != INDEX_NONE)
	{
		StartEnterVehicle(Vehicle, SI);
	}
}

void UCharacterVehicleManager::StartEnterVehicle(AVehicle_Base* Vehicle, int32 SeatIndex)
{
	UpdateCSI(SeatIndex);
	ManageinVehicleStatus(Vehicle, true);
	StartEnterSeat();
}

void UCharacterVehicleManager::CharacterEnterVehicle()
{
	//if Vehicle RC Data does actually become a thing, certain things here need to be blocked based on that new property's value
	if (!GetCurrentVehicle()->VehicleData->bCanRemoteControl)
	{
		PhysicallyEnterVehicle();
	}

	GetOwnerCharacter()->ManageIMC(UBS2FunctionLibrary::GetDataSubsystem(this)->GetCharacterDefaults()->DefaultGameplayIMC.Get(), nullptr, -1);
	if (GetOwnerCharacter()->IsLocalHuman())
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
	ACharacter_Base* Character = GetOwnerCharacter();
	Character->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Ignore);
	Character->GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Vehicle, ECollisionResponse::ECR_Ignore);
	Character->FPArms->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Ignore);
	Character->FPLegs->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Ignore);
	Character->FPLegs->SetHiddenInGame(true);
	for (int32 i = 0; i < GetOwnerCharacter()->LoadoutManager->GetNumWeapons(); i++)
	{
		Character->LoadoutManager->UpdateWeaponCollision(ECC_Vehicle, ECR_Ignore, i);
	}
	Character->LoadoutManager->UpdateWeaponSlotVisibility(Character->LoadoutManager->GetCII(), true);
	for (int32 G = 0; G < GetOwnerCharacter()->LoadoutManager->GetNumGadgets(); G++)
	{
		Character->LoadoutManager->UpdateGadgetCollision(ECC_Vehicle, ECR_Ignore, G);
	}
	Character->GetCharacterMovement()->SetMovementMode(MOVE_None);
	Character->AttachToActor(GetCurrentVehicle(), FAttachmentTransformRules::KeepRelativeTransform);
		
	Character->FPArmsSpringArm->bUsePawnControlRotation = false;
	Character->FPArmsSpringArm->bInheritRoll = true;
	Character->FPArmsSpringArm->bEnableCameraLag = false;
	Character->FPCamera->bUsePawnControlRotation = false;
	Character->FPCamera->SetRelativeRotation(FRotator());
	Character->bUseControllerRotationYaw = false;
	

}

#pragma endregion

#pragma region ExitVehicle

void UCharacterVehicleManager::CharacterExitVehicle()
{
	if (!GetCurrentVehicle())	{ return; }
	
	StartExitSeat(GetCSI());
	
	if (!GetCurrentVehicle()->VehicleData->bCanRemoteControl)
	{
		PhysicallyExitVehicle();
	}

	GetOwnerCharacter()->UpdateViewTarget(GetOwnerCharacter(), GetOwnerCharacter()->FPCamera);

	GetOwnerCharacter()->ManageIMC(nullptr, UBS2FunctionLibrary::GetDataSubsystem(this)->GetCharacterDefaults()->DefaultGameplayIMC.Get(), 1);

	if (GetOwnerCharacter()->LoadoutManager->GetIsCurrentSlotActuallyWeapon() && GetOwnerCharacter()->IsLocalHuman())
	{
		ExitVehicle_LocalPlayer();
	}
	
	CharacterVehicleState = FCharacterVehicleState();
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
	ACharacter_Base* Character = GetOwnerCharacter();
	Character->DetachFromActor(FDetachmentTransformRules::KeepRelativeTransform);

	FVector ExitLocation = CalculateSafeExitLocation(GetCurrentVehicle());
	Character->SetActorLocation(ExitLocation);

	//Character->HandleUpdateStance(ECharacterStance::Standing);
	Character->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Block);
	Character->GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Vehicle, ECollisionResponse::ECR_Block);
	Character->FPLegs->SetHiddenInGame(false);
	Character->GetCharacterMovement()->SetMovementMode(MOVE_Walking);		//make this more dynamic (are we falling out of ejecting from a jet for example)
	Character->LoadoutManager->UpdateWeaponSlotVisibility(Character->LoadoutManager->GetCII(), false);
	Character->FPCamera->bUsePawnControlRotation = true;
	Character->bUseControllerRotationYaw = true;
	Character->FPArmsSpringArm->bUsePawnControlRotation = true;
	Character->FPArmsSpringArm->bInheritRoll = false;
	Character->FPArmsSpringArm->bEnableCameraLag = true;
}

#pragma endregion

#pragma region EnterSeat

void UCharacterVehicleManager::StartEnterSeat()
{
	AVehicle_Base* Vehicle = GetCurrentVehicle();
	const FSeatData& SeatData = GetCurrentSeatData();
	
	Vehicle->SetupNewSeat(GetCSI(), GetOwnerCharacter()->IsLocalHuman());
	switch (GetCurrentSeatDefinition().SeatRole)
	{
		case ESeatRole::DriverGunner:
		case ESeatRole::Gunner:
			if (SeatData.ViewMethod == E_ViewMethod::Windowed)
			{
				Vehicle->VehicleLoadoutManager->WindowedRangefinder.AddDynamic(this, &UCharacterVehicleManager::UpdateRangefinder_WindowedVehicle);
			}
			break;
	}
	HandleViewMethod(GetCSI());
	CharacterEnterSeat(SeatData.DefaultCharacterContext);
}


void UCharacterVehicleManager::CharacterEnterSeat(const FCharacterSeatContext& SeatContext)
{
	//if Vehicle RC Data does actually become a thing, Character transform should not be set (block switch behind if)
	
	if (!GetCurrentVehicle()->VehicleData->bCanRemoteControl)
	{
		switch (GetCurrentSeatDefinition().SeatRole)
		{
			case ESeatRole::Driver:
			case ESeatRole::Passenger:
				GetOwnerCharacter()->SetActorRelativeTransform(SeatContext.SeatTransform);
				break;
			case ESeatRole::DriverGunner:
			case ESeatRole::Gunner:
				HandleEnterSeat_Gunner(SeatContext);
				break;
		}

		if (GetCurrentSeatData().DefaultCharacterContext.SeatABP_3P)
		{
			GetOwnerCharacter()->GetMesh()->GetAnimInstance()->LinkAnimClassLayers(GetCurrentSeatData().DefaultCharacterContext.SeatABP_3P);
		}
		if (GetCurrentSeatData().DefaultCharacterContext.SeatABP_FP)
		{
			GetOwnerCharacter()->FPArms->GetAnimInstance()->LinkAnimClassLayers(GetCurrentSeatData().DefaultCharacterContext.SeatABP_FP);
		}
		GetOwnerCharacter()->UpdateCharacterMeshVisibility(SeatContext.bIsCharacterVisible);
	}
	
	GetOwnerCharacter()->ManageIMC(nullptr, GetCurrentSeatDefinition().InputMappingContext, 1);

	if (GetOwnerCharacter()->IsLocalHuman())
	{
		if (SeatContext.SeatHMD)
		{
			UpdateVehicleHUD(SeatContext.SeatHMD);
		}

		UpdateUI_EnterSeat();
	}
}

void UCharacterVehicleManager::HandleEnterSeat_Gunner(const FCharacterSeatContext& SeatContext)
{
	TObjectPtr<UVehicleLoadoutManager> VWLC = GetCurrentVehicle()->VehicleLoadoutManager;
	const FVehicleWeaponInstanceData& VWID = VWLC->GetVWID(GetCSI(), VWLC->GetCWIForSeat(GetCSI()), VWLC->GetEquippedWeaponIDInSeat(GetCSI()));
	if (VWID.AttachmentInstanceData.bAttachCharacter)
	{
		TWeakObjectPtr<USkeletalMeshComponent> WeaponMesh = VWLC->VehicleWeaponSystem.Find(GetCSI())->SeatWeaponSystemState.WeaponSystemMesh;
		FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, false);
		GetOwnerCharacter()->GetRootComponent()->AttachToComponent(WeaponMesh.Get(), AttachmentRules, FName("Test"));
		GetOwnerCharacter()->SetActorRelativeTransform(VWID.AttachmentInstanceData.CharacterTransform);
	}
	else
	{
		GetOwnerCharacter()->SetActorRelativeTransform(SeatContext.SeatTransform);
	}
}

#pragma endregion

void UCharacterVehicleManager::StartExitSeat(int32 SeatIndex)
{
	const FSeatData& SeatData = GetCurrentVehicle()->GetSeatData(SeatIndex);
	CharacterExitSeat(SeatIndex);
	GetCurrentVehicle()->DropSeat(SeatIndex);
	switch (GetCurrentVehicle()->GetVehicleTypeSeatDefinition(SeatIndex).SeatRole)
	{
		case ESeatRole::DriverGunner:
		case ESeatRole::Gunner:
			if (SeatData.ViewMethod == E_ViewMethod::Windowed)
			{
				GetCurrentVehicle()->VehicleLoadoutManager->WindowedRangefinder.RemoveDynamic(this, &UCharacterVehicleManager::UpdateRangefinder_WindowedVehicle);
			}
			break;
	}
}

void UCharacterVehicleManager::CharacterExitSeat(int32 SeatIndex)
{
	if (!GetCurrentVehicle()->VehicleData->bCanRemoteControl)
	{
		if (GetOwnerCharacter()->GetAttachParentActor()->GetRootComponent() != GetOwnerCharacter()->GetRootComponent()->GetAttachParent())
		{
			GetOwnerCharacter()->GetRootComponent()->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
			GetOwnerCharacter()->AttachToActor(GetCurrentVehicle(), FAttachmentTransformRules::KeepRelativeTransform);
		}
	}
	
	GetOwnerCharacter()->GetMesh()->GetAnimInstance()->LinkAnimClassLayers(nullptr);
	GetOwnerCharacter()->FPArms->GetAnimInstance()->LinkAnimClassLayers(nullptr);

	if (GetOwnerCharacter()->IsLocalHuman())
	{
		UpdateVehicleHUD(nullptr);
	}

	GetOwnerCharacter()->ManageIMC(GetCurrentVehicle()->VehicleTypeDefintion->SeatDefinitions[SeatIndex].InputMappingContext, nullptr, 0);
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

void UCharacterVehicleManager::AttemptChangeSeat()
{
	if (!GetInVehicle())		{ return;}
	int32 SI = GetCurrentVehicle()->FindNextEmptySeat(GetCSI());
	if (SI != INDEX_NONE)
	{
		ChangeSeat(SI);
	}
}

void UCharacterVehicleManager::ChangeSeat(int32 SeatIndex)
{
	int32 LSI = GetCSI();
	UpdateCSI(SeatIndex);
	StartExitSeat(LSI);
	StartEnterSeat();
}

#pragma endregion

void UCharacterVehicleManager::UpdateCSI(int32 NewCSI)
{
	CharacterVehicleState.CSI = NewCSI;
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
	switch (GetCurrentVehicle()->VehicleTypeDefintion->SeatDefinitions[GetCSI()].SeatRole)
	{
		case ESeatRole::Driver:
			UBS2FunctionLibrary::GetHUDSubsystem(this)->UpdateSpeedHUD_Vehicle(GetCurrentVehicle()->GetCurrentSpeed());
			break;
		case ESeatRole::Gunner:
			UpdateUI_EnterSeat_Turrets();
			break;
		case ESeatRole::DriverGunner:
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
	if (GetCurrentSeatDefinition().ControlledTurretIndexes.Num())
	{
		const int32& CTI = GetCurrentVehicle()->GetControlledTurret(GetCSI());
				
		float MinPitch, MaxPitch, CurrentPitch;
		GetCurrentVehicle()->GetTurretPitchRange(CTI, MinPitch, MaxPitch, CurrentPitch);

		UBS2FunctionLibrary::GetHUDSubsystem(this)->HandleTurretPitchUpdate(MinPitch, MaxPitch, CurrentPitch);
	}
}

void UCharacterVehicleManager::OnVehicleHealthChanged()
{
	if (GetOwnerCharacter()->IsLocalHuman())
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

	const FSeatData& OccupiedSeatData = GetCurrentSeatData();
	if (OccupiedSeatData.ViewMethod != E_ViewMethod::Windowed)
	{
		return;
	}

	TWeakObjectPtr<UVehicleLoadoutManager> VWLC = GetCurrentVehicle()->VehicleLoadoutManager;
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

	if (!GetOwnerCharacter()->IsLocalHuman())
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

	FHitResult& HitResult = WeaponSystem->SeatWeaponSystemState.EquippedWeaponState.RaycastData.RangefinderData;
	TObjectPtr<UStaticMeshComponent> Quad = WeaponSystem->SeatWeaponSystemState.ReticleQuad.Get();

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

void UCharacterVehicleManager::HandleViewMethod(int32 SeatIndex)
{
	if (!GetOwnerCharacter()->IsLocalHuman())		{ return; }
	const FVehicleTypeSeatDefinition& SeatTypeData = GetCurrentVehicle()->GetVehicleTypeSeatDefinition(SeatIndex);
	if (SeatTypeData.SeatRole != ESeatRole::DriverGunner && SeatTypeData.SeatRole != ESeatRole::Gunner)
	{
		HandleViewMethod_Default(SeatIndex);
		return;
	}

	GetCurrentVehicle()->SyncActiveCameraForSeat(SeatIndex);
	
	UCameraComponent* WeaponCam = GetCurrentVehicle()->GetSeatWeaponCam(SeatIndex);
	if (!WeaponCam)
	{
		HandleViewMethod_Default(SeatIndex);
		return;
	}
	
	TWeakObjectPtr<AActor> ViewTarget = GetCurrentVehicle()->VehicleLoadoutManager->GetCurrentViewTargetAtSeatIndex(SeatIndex);
	GetOwnerCharacter()->UpdateViewTarget(ViewTarget, WeaponCam);
}

void UCharacterVehicleManager::HandleViewMethod_Default(int32 SeatIndex)
{
	const FSeatData& SeatData = GetCurrentVehicle()->VehicleData->Seats[SeatIndex];
	switch (SeatData.ViewMethod)
	{
		case E_ViewMethod::Windowed:
			GetOwnerCharacter()->UpdateViewTarget(GetOwnerCharacter(), GetOwnerCharacter()->FPCamera);
			break;
		case E_ViewMethod::Remote:
		{
			GetCurrentVehicle()->SyncActiveCameraForSeat(SeatIndex);
			GetOwnerCharacter()->UpdateViewTarget(GetCurrentVehicle(), GetCurrentVehicle()->VehicleCurrentState.SeatStates[SeatIndex].DefaultCamera);
			break;
		}
	}
}

void UCharacterVehicleManager::ToggleCurrentViewMode()
{
	//it is assumed that if this is called this is a local human
	if (!GetInVehicle())	{ return; }
	if (!GetCurrentSeatDefinition().ThirdPersonDefinition.canEnterTP)	{ return; }
	switch (GetCurrentSeatData().ViewMethod)
	{
		case E_ViewMethod::Windowed:
			if (GetOwnerCharacter()->FPCamera->IsActive())
			{
				GetOwnerCharacter()->FPCamera->Deactivate();
				GetOwnerCharacter()->UpdateViewTarget(GetCurrentVehicle(), GetCurrentVehicle()->VehicleCurrentState.SeatStates[GetCSI()].TPCamera);
			}
			else
			{
				GetOwnerCharacter()->UpdateViewTarget(GetOwnerCharacter(), GetOwnerCharacter()->FPCamera);
			}
			break;
		case E_ViewMethod::Remote:
			GetCurrentVehicle()->ToggleRemoteViewMode(GetCSI());	
			break;
	}
}

#pragma region Getters

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

bool UCharacterVehicleManager::GetInVehicle()
{
	bool inVehicle = GetCurrentVehicle() != nullptr && GetCSI() != INDEX_NONE;
	return inVehicle;
}

const FVehicleTypeSeatDefinition& UCharacterVehicleManager::GetCurrentSeatDefinition()
{
	return GetCurrentVehicle()->GetVehicleTypeSeatDefinition(GetCSI());
}

const FSeatData& UCharacterVehicleManager::GetCurrentSeatData()
{
	return GetCurrentVehicle()->GetSeatData(GetCSI());
}

FSeatWeaponSystemState& UCharacterVehicleManager::GetCurrentSeatWeaponSystem()
{
	//assumes in a vehicle in a gunner seat
	return GetCurrentVehicle()->VehicleLoadoutManager->VehicleWeaponSystem.Find(GetCSI())->SeatWeaponSystemState;
}

USkeletalMeshComponent* UCharacterVehicleManager::GetActiveCharacterAttachTarget()
{
	if (GetCurrentSeatDefinition().SeatRole != ESeatRole::DriverGunner && GetCurrentSeatDefinition().SeatRole != ESeatRole::Gunner)
	{
		return GetCurrentVehicle()->VehicleMeshComponent;	//default
	}
	
	const FVehicleWeaponInstanceData& VWID = GetCurrentVehicle()->VehicleLoadoutManager->GetVWID(GetCSI(), GetCurrentVehicle()->VehicleLoadoutManager->GetCWIForSeat(GetCSI()), GetCurrentVehicle()->VehicleLoadoutManager->GetEquippedWeaponIDInSeat(GetCSI()));
	if (VWID.AttachmentInstanceData.bAttachCharacter && GetCurrentSeatData().ViewMethod  == E_ViewMethod::Windowed)
	{
		return GetCurrentSeatWeaponSystem().WeaponSystemMesh.Get();
	}
	
	return GetCurrentVehicle()->VehicleMeshComponent;
}

FName UCharacterVehicleManager::GetIKAttachSocket(bool Foot, bool Left)
{
	FString Bone = Foot ? FString("Foot") : FString("Hand");
	FString Side = Left ? FString("Left") : FString("Right");
	FString InitalString = FString::Printf(TEXT("%s_%s"), *Bone, *Side);
	FName IKAttachSocket = FName(*InitalString);
	const FVehicleWeaponInstanceData& VWID = GetCurrentVehicle()->VehicleLoadoutManager->GetVWID(GetCSI(), GetCurrentVehicle()->VehicleLoadoutManager->GetCWIForSeat(GetCSI()), GetCurrentVehicle()->VehicleLoadoutManager->GetEquippedWeaponIDInSeat(GetCSI()));
	USkeletalMeshComponent* WeaponSystemMesh = GetCurrentSeatWeaponSystem().WeaponSystemMesh.Get();
	if (VWID.AttachmentInstanceData.bAttachCharacter && WeaponSystemMesh)
	{
		return IKAttachSocket;	
	}

	InitalString = FString::Printf(TEXT("%s_%s_%02d"), *Bone, *Side, GetCSI());
	return FName(*InitalString);
}

ACharacter_Base* UCharacterVehicleManager::GetOwnerCharacter()
{
	return Cast<ACharacter_Base>(GetOwner());
}

#pragma endregion


