#include "Core/Weapons/VehicleWeaponLogicComponent.h"
#include "Save/SaveSubsystem.h"
#include "Save/PlayerSave_Loadout.h"
#include "Character_Base.h"	
#include "Core/Weapons/Projectiles/Projectile_Base.h"
#include "Data/Weapons/Data_Projectile.h"
#include "Data/Runtime/ProjectileTypes.h"
#include "Data/Weapons/Data_VehicleWeapon.h"
#include "Data/Weapons/WeaponDefaults.h"
#include "Data/Vehicles/VehicleDefaults.h"
#include "Data/Data_VehicleAttachments.h"
#include "Data/Data_Optics.h"
#include "Core/Weapons/WeaponFunctions.h"
#include "Core/UI/VehicleHUDs/UW_HUD_Vehicle_Base.h"
#include "Utilities/HUDSubsystem.h"
#include "Utilities/I_Anims.h"
#include "Utilities/BS2FunctionLibrary.h"
#include "Camera/CameraActor.h"
#include "Components/AudioComponent.h"
#include "NiagaraComponent.h"
#include "AudioParameterControllerInterface.h"
#include "Vehicle_Base.h"

UVehicleWeaponLogicComponent::UVehicleWeaponLogicComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UVehicleWeaponLogicComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerDataAccessor = Cast<IVehicleDataAccessor>(GetOwner());
}

void UVehicleWeaponLogicComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	HandleSeatRangefinders();
}

#pragma region Factory/Initialization
void UVehicleWeaponLogicComponent::Init_VehicleWeaponSystem(TMap<int32, FSavedSeatLoadout> SeatLoadouts)
{
	const FVehicleData& VehicleData = OwnerDataAccessor->GetVehicleData();
	for (int32 SeatIndex = 0; SeatIndex < VehicleData.Seats.Num(); SeatIndex++)
	{
		if (VehicleData.Seats[SeatIndex].SeatRole == E_SeatRole::DriverGunner || VehicleData.Seats[SeatIndex].SeatRole == E_SeatRole::Gunner)
		{
			Init_WAC(SeatIndex);
			Init_HUDReticleQuad(SeatIndex);

			if (SeatLoadouts.Find(SeatIndex) && SeatLoadouts.Find(SeatIndex)->Weapons.Num() > 0)
			{
				//iterates over every seat in the map and sets up using input seat loadouts (starting loadout from vehicle starting data for instance)
				TArray<FName> SavedWeaponIDs = SeatLoadouts.Find(SeatIndex)->Weapons;
				ApplyWeaponsToSeat(SeatIndex, SavedWeaponIDs);
			}
			else
			{
				//iterates over every seat in the map and sets up USING DEFAULTS
				TArray<FName> DefaultWeaponIDs = VehicleData.Seats[SeatIndex].AvailableItems.GetDefaultWeaponIDs();
				ApplyWeaponsToSeat(SeatIndex, DefaultWeaponIDs);
			}
		}
	}
	Init_Turrets(VehicleData.Turrets.Num());
}

void UVehicleWeaponLogicComponent::Init_HUDReticleQuad(int32 SeatIndex)
{
	if (!OwnerDataAccessor->GetVehicleData().Seats[SeatIndex].DefaultCharacterContext.SeatHUD)
	{
		return;
	}
	UStaticMesh* DefaultPlane = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
	TWeakObjectPtr<UStaticMeshComponent> NewQuad = NewObject<UStaticMeshComponent>(this);
	NewQuad->RegisterComponent();
	NewQuad->AttachToComponent(OwnerDataAccessor->GetVehicleState().SeatStates[SeatIndex].SeatHUDComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
	NewQuad->SetStaticMesh(DefaultPlane);
	NewQuad->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	NewQuad->SetCastShadow(false);
	NewQuad->SetReceivesDecals(false);
	NewQuad->SetRelativeRotation(FRotator(0.f, -90.f, 90.f));
	NewQuad->SetRelativeLocation(FVector(0.1f, 0.f, 0.f));

	UMaterialInterface* MasterMat = UBS2FunctionLibrary::GetDataSubsystem(this)->GetVehicleDefaults()->HUDMasterMaterial.LoadSynchronous();
	UMaterialInstanceDynamic* DynMat = NewQuad->CreateDynamicMaterialInstance(0, MasterMat);		//both creates and assigns

	FVehicleWeaponSystem_Runtime& SWS = *VehicleWeaponSystem.Find(SeatIndex);
	SWS.VehicleWeaponSystemState.ReticleQuad = NewQuad;
}

void UVehicleWeaponLogicComponent::Init_WAC(int32 SeatIndex)
{
	//Initialize Weapon Audio Component
	FVehicleWeaponSystem_Runtime NewSystem;
	TWeakObjectPtr<UAudioComponent> NewAudioComp = NewObject<UAudioComponent>(GetOwner());
	NewAudioComp->SetupAttachment(OwnerDataAccessor->GetVehicleMesh());
	NewAudioComp->RegisterComponent();
	TSoftObjectPtr<UDA_WeaponDefaults> WeaponDefaults = UBS2FunctionLibrary::GetDataSubsystem(this)->WeaponDefaultsDAAsset;
	NewAudioComp->SetSound(WeaponDefaults->WeaponDefaults.DefaultWeaponMetaSound.LoadSynchronous());
	NewSystem.VehicleWeaponSystemState.WeaponAudioComponent = NewAudioComp;
	NewSystem.VehicleWeaponSystemState.WeaponAudioComponent->bAutoActivate = false;		//important so that we dont get the nasty concurrent audio bug (if not firing/in use this doesnt need to be on)
	VehicleWeaponSystem.Add(SeatIndex, NewSystem);
}

void UVehicleWeaponLogicComponent::Init_Turrets(int32 NumOfTurrets)
{
	TurretStates.SetNum(NumOfTurrets);
}

#pragma endregion

#pragma region ApplyWeapons

void UVehicleWeaponLogicComponent::ApplyVWID(int32 SeatIndex, int32 WeaponIndex, FName WeaponID)
{
	UE_LOG(LogTemp, Error, TEXT("[VWLC::ApplyWeaponInstanceDataAtIndexToSeat] SI = %d, WI = %d, WeaponID = %s"), SeatIndex, WeaponIndex, *WeaponID.ToString());
	FVehicleWeaponSystem_Runtime* WeaponSystem = VehicleWeaponSystem.Find(SeatIndex);
	FVehicleWeapon_Runtime& VehicleWeaponToFill = WeaponSystem->Weapons[WeaponIndex];
	VehicleWeaponToFill.VehicleWeaponInstanceData = GetVWID(SeatIndex, WeaponIndex, WeaponID);

	HandleApplyWeaponMesh(SeatIndex, WeaponIndex);

	//now that we have the mesh... do either mount projectile OR cache muzzle sockets
	//mount projectiles
	if (VehicleWeaponToFill.VehicleWeaponInstanceData.bAreProjectilesMounted)
	{
		MountProjectiles(SeatIndex, WeaponIndex);
	}
	//or setup muzzle sockets
	else
	{
		const FVehicleData& VehicleData = OwnerDataAccessor->GetVehicleData();
		FName SeatName = (*VehicleData.Seats[SeatIndex].SeatName.ToString());
		if (VehicleWeaponToFill.VehicleWeaponInstanceData.bHasSeparateMesh)
		{
			SetupMuzzleSockets(WeaponSystem->VehicleWeaponSystemState.WeaponSystemMesh, SeatIndex, WeaponIndex, SeatName);
		}
		else
		{
			SetupMuzzleSockets(OwnerDataAccessor->GetVehicleMesh(), SeatIndex, WeaponIndex, SeatName);
		}
	}

	//everything else that a camera could be mounted on has been set, now safe to mount cam
	if (VehicleWeaponToFill.VehicleWeaponInstanceData.bHasSpecialCam)
	{
		ConfigureWeaponCam(SeatIndex, WeaponIndex, *WeaponSystem);
	}

	//lastly, apply the non-functionally necessary decorative stuff
	HandleApplyWeaponDecoratives(SeatIndex, WeaponIndex, WeaponID);
}

void UVehicleWeaponLogicComponent::ApplyWeaponAtIndexToSeat(int32 SeatIndex, int32 WeaponIndex, FName WeaponID)
{
	FVehicleWeaponSystem_Runtime* WeaponSystem = VehicleWeaponSystem.Find(SeatIndex);
	FVehicleWeapon_Runtime& VehicleWeaponToFill = WeaponSystem->Weapons[WeaponIndex];
	VehicleWeaponToFill.VehicleWeaponInstanceData = GetVWID(SeatIndex, WeaponIndex, WeaponID);
	FWeapon_Runtime& DefaultWeaponDataToFill = VehicleWeaponToFill.VehicleWeaponState.BaseWeaponRuntimeData;

	if (DefaultWeaponDataToFill.WeaponID.IsNone())
	{
		//"none" can still have decoratives so do this regardless of anything if there's any weapon
		if (VehicleWeaponToFill.VehicleWeaponInstanceData.WeaponDecoratives.Num() > 0)
		{
			ClearWeaponDecorativesFromSlot(SeatIndex, WeaponIndex);
		}
	}
	else
	{
		//if theres already a weapon on this slot, clear it first (previous player's or starting/default loadout for example)
		ClearWeaponSlotFromSeat(SeatIndex, WeaponIndex);
	}

	DefaultWeaponDataToFill.WeaponID = WeaponID;
	if (WeaponID.IsNone())
	{
		return;
	}

	ApplyStaticWeaponData(SeatIndex, WeaponIndex, WeaponID);

	//initialize state on weapon side
	//make apply WeaponStateToSeat function?
	const FVehicleWeaponData& VehicleWeaponDataToUse = *UBS2FunctionLibrary::GetDataSubsystem(this)->GetVehicleWeaponDataRow(WeaponID);
	DefaultWeaponDataToFill.WeaponState.CurrentFireMode = VehicleWeaponDataToUse.WeaponData.WeaponFunctionality.DefaultFireMode;
	DefaultWeaponDataToFill.WeaponState.CurrentAmmoinMag = VehicleWeaponDataToUse.WeaponData.AmmoData.MagSize;
	DefaultWeaponDataToFill.WeaponState.CurrentReserveAmmo = VehicleWeaponDataToUse.WeaponData.AmmoData.MaxReserveAmmo;

	ApplyVWID(SeatIndex, WeaponIndex, WeaponID);
}

void UVehicleWeaponLogicComponent::ApplyStaticWeaponData(int32 SeatIndex, int32 WeaponIndex, FName WeaponID)
{
	const FVehicleWeaponData& VehicleWeaponDataToUse = *UBS2FunctionLibrary::GetDataSubsystem(this)->GetVehicleWeaponDataRow(WeaponID);
	const FBaseWeaponData* BaseWeaponData = &VehicleWeaponDataToUse.WeaponData;
	TArray<const FBaseWeaponData*>& BaseWeaponDataArray = CurrentVehicleBaseWeaponData.FindOrAdd(SeatIndex);
	if (!BaseWeaponDataArray.IsValidIndex(WeaponIndex))
	{
		BaseWeaponDataArray.SetNum(WeaponIndex + 1);
	}
	BaseWeaponDataArray[WeaponIndex] = BaseWeaponData;
}

void UVehicleWeaponLogicComponent::HandleApplyWeaponMesh(int32 SeatIndex, int32 WeaponIndex)
{
	FVehicleWeaponSystem_Runtime* WeaponSystem = VehicleWeaponSystem.Find(SeatIndex);
	FVehicleWeapon_Runtime& VehicleWeaponToFill = WeaponSystem->Weapons[WeaponIndex];
	if (VehicleWeaponToFill.VehicleWeaponInstanceData.bHasSeparateMesh && WeaponIndex == OwnerDataAccessor->GetVehicleData().Seats[SeatIndex].AvailableItems.WeaponMeshDriverSlotIndex)
	{
		const FVehicleAttachmentData& AttachmentData = *UBS2FunctionLibrary::GetDataSubsystem(this)->GetVehicleAttachmentDataRow(VehicleWeaponToFill.VehicleWeaponInstanceData.AttachmentInstanceData.AttachmentID);
		TWeakObjectPtr<USkeletalMesh> LoadedMesh = AttachmentData.Attachment_SKM.LoadSynchronous();
		UClass* LoadedAnimClass = AttachmentData.Attachment_AnimClass.LoadSynchronous();
		if (!WeaponSystem->VehicleWeaponSystemState.WeaponSystemMesh.IsValid())
		{
			// Create it for the first time
			WeaponSystem->VehicleWeaponSystemState.WeaponSystemMesh = ApplyWeaponMeshToVehicle(AttachmentData.Attachment_SKM.Get(), SeatIndex, VehicleWeaponToFill.VehicleWeaponInstanceData.AttachmentInstanceData.AttachmentTransform);
		}
		UpdateWeaponAttachment(*WeaponSystem, LoadedMesh, LoadedAnimClass, VehicleWeaponToFill.VehicleWeaponInstanceData.AttachmentInstanceData.AttachmentTransform, VehicleWeaponToFill.VehicleWeaponInstanceData.AttachmentInstanceData.AttachmentID);

		//apply camo
		FName CurrentVehicleCamo = OwnerDataAccessor->GetVehicleState().GenericVehicleState.CurrentCamo;
		if (CurrentVehicleCamo != NAME_None)
		{
			OwnerDataAccessor->GetVehicle().ApplyCamoToAttachment(WeaponSystem->VehicleWeaponSystemState.WeaponSystemMesh, WeaponSystem->VehicleWeaponSystemState.WSAttachmentID, CurrentVehicleCamo);
		}
	}
	else if (WeaponSystem->VehicleWeaponSystemState.WeaponSystemMesh.IsValid())
	{
		// If the driving slot is changed to a weapon with NO mesh, null the existing mesh
		WeaponSystem->VehicleWeaponSystemState.WeaponSystemMesh->SetSkeletalMesh(nullptr);
		WeaponSystem->VehicleWeaponSystemState.WSAttachmentID = NAME_None;
	}
}

void UVehicleWeaponLogicComponent::MountProjectiles(int32 SeatIndex, int32 WeaponIndex)
{
	USkeletalMeshComponent* VehicleMeshComponent = OwnerDataAccessor->GetVehicleMesh();
	FVehicleWeapon_Runtime& SeatWeaponToFill = VehicleWeaponSystem.Find(SeatIndex)->Weapons[WeaponIndex];
	FWeapon_Runtime& WeaponDataToFill = SeatWeaponToFill.VehicleWeaponState.BaseWeaponRuntimeData;
	const FVehicleWeaponData* VehicleWeaponRow = UBS2FunctionLibrary::GetDataSubsystem(this)->GetVehicleWeaponDataRow(WeaponDataToFill.WeaponID);
	const int32& MagSize = FMath::Min(VehicleWeaponRow->WeaponData.AmmoData.MagSize, WeaponDataToFill.WeaponState.CurrentAmmoinMag);
	const FName& MunitionID = VehicleWeaponRow->WeaponData.WeaponFirePerformance.MunitionID;
	if (!VehicleWeaponRow || MunitionID.IsNone())
	{
		return;
	}

	for (int32 WRM = 0; WRM < MagSize; WRM++)
	{
		FString SocketNameString = FString::Printf(TEXT("WRM_%02d_%02d_%02d"), SeatIndex, WeaponIndex, WRM);		//WRM_SeatIndex_WeaponIndex_ProjectileIndex
		FName SocketName = FName(*SocketNameString);

		FTransform SocketTransform = VehicleMeshComponent->GetSocketTransform(SocketName, RTS_World);	//transform offset is data (is that needed?)
		TWeakObjectPtr<AProjectile_Base> NewProjectile = UBS2FunctionLibrary::GetProjectileSystem(this)->AcquireProjectileFromPool(MunitionID);
		NewProjectile->SetRuntimeContext(VehicleMeshComponent, SocketName);
		SeatWeaponToFill.VehicleWeaponState.CurrentMountedProjectiles.Add(NewProjectile);
		//DONT INITIALIZE, PROJECTILES SHOULD ALREADY BE INITIALIZED BY POOL SUBYSTEM
	}
}

void UVehicleWeaponLogicComponent::ApplyWeaponDecoratives(const TArray<FDecorative>& WeaponDecoratives, FVehicleWeapon_Runtime& RuntimeWeaponData)
{
	RuntimeWeaponData.VehicleWeaponState.VehicleWeaponDecoratives.SetNum(WeaponDecoratives.Num());
	for (int32 i = 0; i < WeaponDecoratives.Num(); i++)
	{
		const FDecorative& WeaponDecorative = WeaponDecoratives[i];
		if (WeaponDecorative.AttachmentID != NAME_None)
		{
			const FVehicleAttachmentData& AttachmentData = *UBS2FunctionLibrary::GetDataSubsystem(this)->GetVehicleAttachmentDataRow(WeaponDecorative.AttachmentID);
			TWeakObjectPtr<UStaticMesh> LoadedMesh = AttachmentData.Attachment_SM.LoadSynchronous();
			TWeakObjectPtr<UStaticMeshComponent> SMComp = NewObject<UStaticMeshComponent>(GetOwner());
			SMComp->SetStaticMesh(LoadedMesh.Get());
			SMComp->SetupAttachment(OwnerDataAccessor->GetVehicleMesh(), WeaponDecorative.SocketName);
			SMComp->SetRelativeTransform(WeaponDecorative.TransformOffset);
			SMComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Vehicle, ECollisionResponse::ECR_Ignore);
			SMComp->RegisterComponent();
			RuntimeWeaponData.VehicleWeaponState.VehicleWeaponDecoratives[i].DecorativeMesh = SMComp;
			RuntimeWeaponData.VehicleWeaponState.VehicleWeaponDecoratives[i].DecorativeAttachmentID = WeaponDecorative.AttachmentID;

			FName CurrentVehicleCamo = OwnerDataAccessor->GetVehicleState().GenericVehicleState.CurrentCamo;
			if (CurrentVehicleCamo != NAME_None)
			{
				OwnerDataAccessor->GetVehicle().ApplyCamoToAttachment(SMComp, WeaponDecorative.AttachmentID, CurrentVehicleCamo);
			}
		}
	}
}

void UVehicleWeaponLogicComponent::ApplyWeaponsToSeat(int32 SeatIndex, TArray<FName> WeaponIDs)
{
	//assumes WeaponIDs are in correct order
	int32 NumWeapons = WeaponIDs.Num();
	check(VehicleWeaponSystem.Find(SeatIndex));
	VehicleWeaponSystem.Find(SeatIndex)->Weapons.SetNum(NumWeapons);

	for (int32 WeaponIndex = 0; WeaponIndex < NumWeapons; WeaponIndex++)
	{
		ApplyWeaponAtIndexToSeat(SeatIndex, WeaponIndex, WeaponIDs[WeaponIndex]);
	}
}

void UVehicleWeaponLogicComponent::ApplySavedWeaponsToSeat(int32 SeatIndex)
{
	//apply SAVED weapons to seat (consider changing the function name)
	USaveSubsystem* SaveSys = GetWorld()->GetGameInstance()->GetSubsystem<USaveSubsystem>();
	const FVehicleData& VehicleData = OwnerDataAccessor->GetVehicleData();
	const FSavedSeatLoadout& SeatLoadoutSave = SaveSys->GetSeatLoadout(VehicleData.Vehicle_Type, SeatIndex);

	ApplyWeaponsToSeat(SeatIndex, SeatLoadoutSave.Weapons);
}

USkeletalMeshComponent* UVehicleWeaponLogicComponent::ApplyWeaponMeshToVehicle(USkeletalMesh* Mesh, int32 SeatIndex, FTransform MeshTransform)
{
	FString SocketNameString = FString::Printf(TEXT("WM_%02d"), SeatIndex);		//WM_SeatIndex	[WM_00]
	FName SocketName = FName(*SocketNameString);

	USkeletalMeshComponent* WeaponComp = NewObject<USkeletalMeshComponent>(GetOwner());
	WeaponComp->SetupAttachment(OwnerDataAccessor->GetVehicleMesh(), SocketName);
	WeaponComp->RegisterComponent(); 

	return WeaponComp;
}

void UVehicleWeaponLogicComponent::UpdateWeaponAttachment(FVehicleWeaponSystem_Runtime& WeaponSystem, TWeakObjectPtr<USkeletalMesh> LoadedMesh, UClass* LoadedAnimClass, FTransform Transform, FName AttachmentID)
{
	WeaponSystem.VehicleWeaponSystemState.WeaponSystemMesh->SetSkeletalMesh(LoadedMesh.Get());
	WeaponSystem.VehicleWeaponSystemState.WeaponSystemMesh->SetAnimInstanceClass(LoadedAnimClass);
	WeaponSystem.VehicleWeaponSystemState.WeaponSystemMesh->SetRelativeTransform(Transform);
	WeaponSystem.VehicleWeaponSystemState.WSAttachmentID = AttachmentID;
}

void UVehicleWeaponLogicComponent::HandleApplyWeaponDecoratives(int32 SeatIndex, int32 WeaponIndex, FName WeaponID)
{
	FVehicleWeaponSystem_Runtime* WeaponSystem = VehicleWeaponSystem.Find(SeatIndex);
	FVehicleWeapon_Runtime& VehicleWeaponToFill = WeaponSystem->Weapons[WeaponIndex];
	FVehicleWeapon_Runtime& SeatWeaponToFill = VehicleWeaponSystem.Find(SeatIndex)->Weapons[WeaponIndex];
	if (VehicleWeaponToFill.VehicleWeaponInstanceData.WeaponDecoratives.Num() > 0)
	{
		//clear any that might be there already
		ClearWeaponDecorativesFromSlot(SeatIndex, WeaponIndex);
	}

	ApplyWeaponDecoratives(VehicleWeaponToFill.VehicleWeaponInstanceData.WeaponDecoratives, VehicleWeaponToFill);
}

void UVehicleWeaponLogicComponent::ConfigureWeaponCam(int32 SeatIndex, int32 WeaponIndex, FVehicleWeaponSystem_Runtime& WeaponSystem)
{
	//setup/initialization of the special weapon turret cam
	//spring arm?
	AActor* OwningActor = GetOwner();
	FString WCNameString = FString::Printf(TEXT("WC_%02d_%02d"), SeatIndex, WeaponIndex);		//WC_SeatIndex_WeaponIndex	[WC_00_00]
	FName WCSocketName = FName(*WCNameString);
	TWeakObjectPtr<USceneComponent> TargetParent = nullptr;
	//UCameraComponent* WeaponCam = NewObject<UCameraComponent>(OwningActor, WCSocketName);
	//create cam comp doesnt work
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	ACameraActor* WeaponCameraActor = GetWorld()->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	switch (WeaponSystem.Weapons[WeaponIndex].VehicleWeaponInstanceData.WeaponCamBehavior.MountMethod)
	{
		case EVehicleWeaponCamMountMethod::VehicleMesh:
			TargetParent = OwnerDataAccessor->GetVehicleMesh();
			break;
		case EVehicleWeaponCamMountMethod::WeaponMesh:
			//cam comp doesnt work (cuz we're attaching it to a component... thats on a component i guess)
			TargetParent = WeaponSystem.VehicleWeaponSystemState.WeaponSystemMesh;
			break;
		case EVehicleWeaponCamMountMethod::MountedProjectile:
			//uses the first projectile in the list, used for TV Guided missile and sort where there should be a mag size of only 1
			TargetParent = WeaponSystem.Weapons[WeaponIndex].VehicleWeaponState.CurrentMountedProjectiles[0]->ProjectileMeshComponent;
			break;
	}

	WeaponCameraActor->AttachToComponent(TargetParent.Get(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WCSocketName);
	WeaponSystem.Weapons[WeaponIndex].VehicleWeaponState.WeaponTurretCamera = WeaponCameraActor;
	WeaponCameraActor->GetCameraComponent()->SetActive(false);
	WeaponCameraActor->GetCameraComponent()->SetHiddenInGame(false);

	AVehicle_Base& Vehicle = OwnerDataAccessor->GetVehicle();
	if (WeaponIndex == GetCWIForSeat(SeatIndex))		
	{
		//NOT THE DEFAULT CAM, NO SPECIAL WEAPON CAM SHOULD BE THE DEFAULT CAM
		//if weapon index = currentweaponindex, we make this the active cam
		Vehicle.UpdateSeatActiveCamera(SeatIndex, WeaponCameraActor->GetCameraComponent());		

		Vehicle.UpdateRemoteActiveCamPP(SeatIndex, UBS2FunctionLibrary::GetDataSubsystem(this)->GetOpticDataRow(OwnerDataAccessor->GetVehicleState().SeatStates[SeatIndex].OpticState.CurrentAvailableOptics[OwnerDataAccessor->GetVehicleState().SeatStates[SeatIndex].OpticState.CurrentOpticIndex])->OpticPPSettings, 1.0f, WeaponCameraActor->GetCameraComponent());
	}
}

#pragma endregion

#pragma region ClearWeapons

void UVehicleWeaponLogicComponent::ClearWeaponDecorativesFromSlot(int32 SeatIndex, int32 WeaponIndex)
{
	FVehicleWeaponSystem_Runtime* VWS = VehicleWeaponSystem.Find(SeatIndex);
	FVehicleWeapon_Runtime& WeaponSlotToClear = VWS->Weapons[WeaponIndex];
	for (int32 i = 0; i < WeaponSlotToClear.VehicleWeaponState.VehicleWeaponDecoratives.Num(); i++)
	{
		auto& Decorative = WeaponSlotToClear.VehicleWeaponState.VehicleWeaponDecoratives[i];

		// Check if the component pointer is a "wild" value (non-null but garbage)
		TWeakObjectPtr<UStaticMeshComponent> MeshPtr = Decorative.DecorativeMesh;

		if (MeshPtr.IsValid())
		{
			MeshPtr->SetStaticMesh(nullptr);
			MeshPtr->DestroyComponent();
		}
		WeaponSlotToClear.VehicleWeaponState.VehicleWeaponDecoratives[i].DecorativeMesh = nullptr;
		WeaponSlotToClear.VehicleWeaponState.VehicleWeaponDecoratives[i].DecorativeAttachmentID = NAME_None;
	}
	WeaponSlotToClear.VehicleWeaponState.VehicleWeaponDecoratives.Empty();
}

void UVehicleWeaponLogicComponent::ClearWeaponSlotFromSeat(int32 SeatIndex, int32 WeaponIndex)
{
	if (!VehicleWeaponSystem.Find(SeatIndex)->Weapons.IsValidIndex(WeaponIndex))
	{
		return;
	}

	FVehicleWeaponSystem_Runtime* VWS = VehicleWeaponSystem.Find(SeatIndex);
	FVehicleWeapon_Runtime& WeaponSlotToClear = VWS->Weapons[WeaponIndex];

	//do the same things as ApplyWeaponInstanceDataAtIndexToSeat... but opposite and in reverse
	if (WeaponSlotToClear.VehicleWeaponState.WeaponTurretCamera)
	{
		WeaponSlotToClear.VehicleWeaponState.WeaponTurretCamera->Destroy();
		WeaponSlotToClear.VehicleWeaponState.WeaponTurretCamera = nullptr;
	}

	//get rid of the current weapon mesh
	if (WeaponIndex == OwnerDataAccessor->GetVehicleData().Seats[SeatIndex].AvailableItems.WeaponMeshDriverSlotIndex && VWS->VehicleWeaponSystemState.WeaponSystemMesh.IsValid())
	{
		//if any cameras/weapons are dependent on this mesh and equipped in that moment... stuff will break/crash
		//how to make better?
		VWS->VehicleWeaponSystemState.WeaponSystemMesh->SetSkeletalMesh(nullptr);
	}

	//ClearMountedProjectiles
	if (WeaponSlotToClear.VehicleWeaponState.CurrentMountedProjectiles.Num() > 0)
	{
		for (int32 i = 0; i < WeaponSlotToClear.VehicleWeaponState.CurrentMountedProjectiles.Num(); i++)
		{
			TWeakObjectPtr<AProjectile_Base> Projectile = WeaponSlotToClear.VehicleWeaponState.CurrentMountedProjectiles[i];
			UBS2FunctionLibrary::GetProjectileSystem(this)->ReturnProjectileToPool(WeaponSlotToClear.VehicleWeaponState.CurrentMountedProjectiles[i]);
			WeaponSlotToClear.VehicleWeaponState.CurrentMountedProjectiles[i]->SetRuntimeContext(nullptr, FName());
			WeaponSlotToClear.VehicleWeaponState.CurrentMountedProjectiles[i] = nullptr;
		}
		WeaponSlotToClear.VehicleWeaponState.CurrentMountedProjectiles.Empty();
	}

	ClearWeaponDecorativesFromSlot(SeatIndex, WeaponIndex);

	WeaponSlotToClear = FVehicleWeapon_Runtime();
}

void UVehicleWeaponLogicComponent::ClearWeaponSystemFromSeat(int32 SeatIndex, bool RemoveFromMap)
{

	FVehicleWeaponSystem_Runtime* WeaponSystemToClear = VehicleWeaponSystem.Find(SeatIndex);
	if (!WeaponSystemToClear || WeaponSystemToClear->Weapons.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("[VehicleWeaponSystemComponent::ClearWeaponSystemFromSeat] WeaponSystem at seat %d is null"), SeatIndex);
		return;
	}
	for (int32 i = 0; i < WeaponSystemToClear->Weapons.Num(); i++)
	{
		ClearWeaponSlotFromSeat(SeatIndex, i);
	}
	if (WeaponSystemToClear->VehicleWeaponSystemState.WeaponSystemMesh.IsValid())
	{
		WeaponSystemToClear->VehicleWeaponSystemState.WeaponSystemMesh->SetSkeletalMesh(nullptr);
	}

	WeaponSystemToClear->Weapons.Empty();

	if (RemoveFromMap)
	{
		VehicleWeaponSystem.Remove(SeatIndex);		//CAUSED CUSTOMIZATION SWITCH VEHICLE TYPE BUG SINCE WEN REMOVED IT MAKES THE MAP GARBAGE MEMORY
	}				
}

void UVehicleWeaponLogicComponent::ClearEntireWeaponSystemFromVehicle()
{
	if (!OwnerDataAccessor)
	{
		// Try to recover the interface from the owner of this component
		OwnerDataAccessor = Cast<IVehicleDataAccessor>(GetOwner());
	}
	const FVehicleData& VehicleData = OwnerDataAccessor->GetVehicleData();
	for (auto& SeatWeaponSystem : VehicleWeaponSystem)	//for each weapon system in the vehicle
	{
		int32& SeatIndex = SeatWeaponSystem.Key;
		ClearWeaponSystemFromSeat(SeatIndex, false);
	}

	VehicleWeaponSystem.Empty();

	if (VehicleWeaponSystem.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("[VehicleWeaponLogicComponent::ClearEntireWeaponSytemFromVehicle] weapon system cleared"));
		OnVWSCleared.Broadcast();
	}
}

#pragma endregion

# pragma region Turrets
float UVehicleWeaponLogicComponent::CalculateTurret(float InputValue, float TurretSpeed, float CurrentTurretValue)
{
	float NewTurretValue = InputValue * TurretSpeed + CurrentTurretValue;
	return NewTurretValue;
}

void UVehicleWeaponLogicComponent::ControlTurret(FVector2D InputValue, int32 SeatIndex)
{
	const FVehicleData& VehicleData = OwnerDataAccessor->GetVehicleData();
	if (!VehicleData.Seats[SeatIndex].AvailableItems.ControlledTurretIndexes.IsValidIndex(0))
	{
		return;
	}
	const int32 ControlledTurretIndex = VehicleData.Seats[SeatIndex].AvailableItems.ControlledTurretIndexes[0];
	const FTurretData& TurretData = VehicleData.Turrets[ControlledTurretIndex];
	FTurretState& TurretState = TurretStates[ControlledTurretIndex];
	float PreviousTurretRotation = TurretState.CurrentTurretRotation;
	float PreviousTurretPitch = TurretState.CurrentTurretPitch;
	float PitchMin = 0.0f;
	float PitchMax = 0.0f;

	//rotation
	float RotRaw = CalculateTurret(InputValue.X, TurretData.TurretRotation.TurretSpeed, TurretState.CurrentTurretRotation);

	const bool RotHasMin = TurretData.TurretRotation.TurretMinMax.HasLowerBound();
	const bool RotHasMax = TurretData.TurretRotation.TurretMinMax.HasUpperBound();

	float NewTurretRotation;

	if (RotHasMin || RotHasMax)
	{
		float Min = TurretData.TurretRotation.TurretMinMax.GetLowerBoundValue();
		float Max = TurretData.TurretRotation.TurretMinMax.GetUpperBoundValue();
		NewTurretRotation = FMath::Clamp(RotRaw, Min, Max);
	}
	else
	{
		NewTurretRotation = FMath::UnwindDegrees(RotRaw);
	}

	//pitch
	float PitchRaw = CalculateTurret(InputValue.Y, TurretData.TurretPitch.TurretSpeed, TurretState.CurrentTurretPitch);

	const bool PitchHasMin = TurretData.TurretPitch.TurretMinMax.HasLowerBound();
	const bool PitchHasMax = TurretData.TurretPitch.TurretMinMax.HasUpperBound();

	float NewTurretPitch;

	if (PitchHasMin || PitchHasMax)
	{
		PitchMin = TurretData.TurretPitch.TurretMinMax.GetLowerBoundValue();
		PitchMax = TurretData.TurretPitch.TurretMinMax.GetUpperBoundValue();
		NewTurretPitch = FMath::Clamp(PitchRaw, PitchMin, PitchMax);
	}
	else
	{
		NewTurretPitch = PitchRaw;
	}

	// Update state
	TurretState.CurrentTurretRotation = NewTurretRotation;
	TurretState.CurrentTurretPitch = NewTurretPitch;

	//if separate mesh, update mesh/mesh's anim bp
	UpdateTurretMesh(SeatIndex, NewTurretRotation, NewTurretPitch);

	UpdateTurretCam(SeatIndex, NewTurretRotation, NewTurretPitch);

	//Update UI
	if (PreviousTurretRotation != NewTurretRotation)
	{
		if (OwnerDataAccessor->GetVehicleState().SeatStates[SeatIndex].UpdateHUD)
		{
			if (OwnerDataAccessor->GetVehicleState().SeatStates[SeatIndex].ActiveCamera)
			{
				GetHUDSystem()->HandleTurretRotationUpdate(OwnerDataAccessor->GetVehicleState().SeatStates[SeatIndex].ActiveCamera->GetComponentRotation().Yaw);
			}
		}
	}
	if (PreviousTurretPitch != NewTurretPitch)
	{
		if (OwnerDataAccessor->GetVehicleState().SeatStates[SeatIndex].UpdateHUD)
		{
			GetHUDSystem()->HandleTurretPitchUpdate(PitchMin, PitchMax, NewTurretPitch);
		}
	}
}

void UVehicleWeaponLogicComponent::UpdateTurretMesh(int32 SeatIndex, float TurretRotation, float TurretPitch)
{
	//this should only be called if the equipped weapon for the seat is a separate mesh
	int32& CWI = GetCWIForSeat(SeatIndex); 
	if (VehicleWeaponSystem.Find(SeatIndex)->Weapons[CWI].VehicleWeaponInstanceData.bHasSeparateMesh)
	{
		UAnimInstance* AnimInst = VehicleWeaponSystem.Find(SeatIndex)->VehicleWeaponSystemState.WeaponSystemMesh->GetAnimInstance();
		if (AnimInst->GetClass()->ImplementsInterface(UAnims::StaticClass()))
		{
			IAnims::Execute_OnUpdateTurret(AnimInst, TurretRotation, TurretPitch);
		}
	}
}

void UVehicleWeaponLogicComponent::UpdateTurretCam(int32 SeatIndex, float TurretRotation, float TurretPitch)
{
	int32& CWI = GetCWIForSeat(SeatIndex);
	FVehicleWeapon_Runtime& CurrentVehicleWeapon = GetEquippedWeaponInSeat(SeatIndex);
	FWeapon_Runtime& CurrentWeapon = CurrentVehicleWeapon.VehicleWeaponState.BaseWeaponRuntimeData;
	const FVehicleWeaponInstanceData& VWID = GetVWID(SeatIndex, CWI, CurrentWeapon.WeaponID);
	FVehicleWeaponSystem_Runtime& SWS = *VehicleWeaponSystem.Find(SeatIndex);

	//only works for a special weapon cam thats not on an attacment and is mounted to the vehicle mesh
	if (!VWID.bHasSeparateMesh && VWID.bHasSpecialCam && VWID.WeaponCamBehavior.MountMethod == EVehicleWeaponCamMountMethod::VehicleMesh)
	{
		UCameraComponent* ActiveCam = OwnerDataAccessor->GetVehicle().GetRemoteActiveCam(SeatIndex);
		ActiveCam->SetRelativeRotation(FRotator(TurretPitch, TurretRotation, 0.0f));
	}
}

#pragma endregion

#pragma region Rangefinder&AimDirection&LineOfConvergence

void UVehicleWeaponLogicComponent::HandleSeatRangefinders()
{
	const FVehicleData& VehicleData = OwnerDataAccessor->GetVehicleData();
	for (auto& SeatWeaponSystem : VehicleWeaponSystem)	//for each weapon system in the vehicle
	{
		int32& SeatIndex = SeatWeaponSystem.Key;
		const FSeatState& CurrentSeatState = OwnerDataAccessor->GetVehicleState().SeatStates[SeatIndex];
		if (!CurrentSeatState.isOccupied)
		{
			continue;
		}

		int32& WeaponIndex = SeatWeaponSystem.Value.VehicleWeaponSystemState.EquippedWeaponState.CurrentWeaponIndex;

		//UPDATE LINE TRACE DATA (hit result)
		const FSeatData& SeatData = VehicleData.Seats[SeatIndex];
		if (SeatData.ViewMethod == E_ViewMethod::Remote || SeatWeaponSystem.Value.Weapons[WeaponIndex].VehicleWeaponInstanceData.bHasSpecialCam)		//if remote or specialweapon cam, do rangefinder
		{
			FTransform CamTransform = CurrentSeatState.ActiveCamera->GetComponentTransform();
			UpdateSeatRangefinder(SeatIndex, CamTransform, {});
		}
		else
		{
			WindowedRangefinder.Broadcast();
		}

		//Handle Calculate Aim Direction
		FHitResult& HitResult = SeatWeaponSystem.Value.VehicleWeaponSystemState.EquippedWeaponState.RaycastData.RangefinderData;
		if (!SeatWeaponSystem.Value.Weapons[WeaponIndex].VehicleWeaponInstanceData.bAreProjectilesMounted)
		{
			//GET MUZZLE SOCKET LOCATION, CALCULATE AIM DIRECTION (line from muzzle to whatever HitResult data)
			if (SeatWeaponSystem.Value.Weapons[WeaponIndex].VehicleWeaponInstanceData.bHasSeparateMesh)
			{
				//separate mesh
				CalculateAimDirection(SeatWeaponSystem.Value.VehicleWeaponSystemState.WeaponSystemMesh, HitResult, WeaponIndex, SeatIndex);
			}
			else
			{
				//vehicle mesh
				CalculateAimDirection(OwnerDataAccessor->GetVehicleMesh(), HitResult, WeaponIndex, SeatIndex);
			}
		}
	}
}

void UVehicleWeaponLogicComponent::UpdateSeatRangefinder(int32 SeatIndex, FTransform TraceTransform, TArray<AActor*> ActorsToIgnore)
{
	const FSeatState& CurrentSeatState = OwnerDataAccessor->GetVehicleState().SeatStates[SeatIndex];
	const FSeatData& SeatData = OwnerDataAccessor->GetVehicleData().Seats[SeatIndex];
	FVehicleWeaponSystem_Runtime* SystemPtr = VehicleWeaponSystem.Find(SeatIndex);
	const FBaseWeaponData& StaticWeaponData = GetBaseWeaponDataInSlot(SeatIndex, GetCWIForSeat(SeatIndex));
	FVehicleWeapon_Runtime& CurrentWeapon = GetEquippedWeaponInSeat(SeatIndex);
	FHitResult& HitResult = SystemPtr->VehicleWeaponSystemState.EquippedWeaponState.RaycastData.RangefinderData;

	if (StaticWeaponData.WeaponFunctionality.HomingFunctionality.HomingCapability != EHomingCapability::NoHoming)
	{
		HandleHoming(SeatIndex, TraceTransform, ActorsToIgnore);
	}
	else
	{
		bool bHit;
		bHit = UWeaponFunctions::PerformWeaponLineTrace(this, TraceTransform, HitResult, ActorsToIgnore);
	}

	if (OwnerDataAccessor->GetVehicleState().SeatStates[SeatIndex].UpdateHUD)
	{
		//HMD
		GetHUDSystem()->UpdateRangefinderHUD_Vehicle(HitResult.Distance / 100);	//distance in meter

		//HUD
	}
}

void UVehicleWeaponLogicComponent::CalculateAimDirection(TWeakObjectPtr<USkeletalMeshComponent> Mesh, FHitResult HitResult, int32 WeaponIndex, int32 SeatIndex)
{
	FVehicleWeaponSystem_Runtime& SeatWeaponSystem = *VehicleWeaponSystem.Find(SeatIndex);
	int32& CWI = GetCWIForSeat(SeatIndex);
	FVehicleWeapon_Runtime& VehicleWeapon = SeatWeaponSystem.Weapons[CWI];
	if (!VehicleWeapon.VehicleWeaponInstanceData.bAreProjectilesMounted)		//arent we already check this isnt true before calling this function?
	{
		auto& MuzzleSockets = VehicleWeapon.VehicleWeaponState.MuzzleSockets;
		auto& AimDirections = SeatWeaponSystem.VehicleWeaponSystemState.EquippedWeaponState.RaycastData.MuzzleAimDirections;

		// CRITICAL: Ensure the AimDirections array matches the Sockets array
		if (AimDirections.Num() != MuzzleSockets.Num())
		{
			AimDirections.SetNum(MuzzleSockets.Num());
		}

		for (int32 MI = 0; MI < SeatWeaponSystem.Weapons[WeaponIndex].VehicleWeaponState.MuzzleSockets.Num(); MI++)
		{
			//get muzzle socket location at muzzle index
			if (!SeatWeaponSystem.Weapons[WeaponIndex].VehicleWeaponState.MuzzleSockets.IsValidIndex(MI))
			{
				return;
			}
			FName MuzzleSocketName = SeatWeaponSystem.Weapons[WeaponIndex].VehicleWeaponState.MuzzleSockets[MI];
			FVector MuzzleLocation = UWeaponFunctions::GetMuzzleTransform(MuzzleSocketName, Mesh).GetLocation();

			//CALCULATE AIM DIRECTION
			AimDirections[MI] = UWeaponFunctions::CalculateAimDirection(HitResult, MuzzleLocation);

			// DEBUG: Draw the convergence line
			UWeaponFunctions::Debug_ProjectilePath(GetWorld(), MuzzleLocation, HitResult);
		}
	}
	else
	{

	}
}

#pragma endregion

#pragma region Homing&LockOn

void UVehicleWeaponLogicComponent::HandleHoming(int32 SeatIndex, FTransform TraceTransform, TArray<AActor*> ActorsToIgnore)
{
	//remember, this gets called on tick by the rangefinder
	FVehicleWeaponSystem_Runtime* SystemPtr = VehicleWeaponSystem.Find(SeatIndex);
	int32& WeaponIndex = GetCWIForSeat(SeatIndex);
	const FBaseWeaponData& StaticWeaponData = GetBaseWeaponDataInSlot(SeatIndex, WeaponIndex);
	const FWeaponHomingData& StaticHomingData = StaticWeaponData.WeaponFunctionality.HomingFunctionality;
	FVehicleWeapon_Runtime& CurrentWeapon = GetEquippedWeaponInSeat(SeatIndex);
	FHitResult& HitResult = SystemPtr->VehicleWeaponSystemState.EquippedWeaponState.RaycastData.RangefinderData;

	bool bHit;
	bHit = UWeaponFunctions::PerformWeaponSphereTrace(this, TraceTransform, HitResult, ActorsToIgnore, 35.0f);
	FLockOnState& LockOnState = CurrentWeapon.VehicleWeaponState.BaseWeaponRuntimeData.WeaponState.LockOnState;

	switch (StaticHomingData.HomingCapability)
	{
		case EHomingCapability::RequireLockOn:
		case EHomingCapability::CanLockOn:
			HandleLockOn(SeatIndex, WeaponIndex);
			break;
		case EHomingCapability::WireGuided1:
		case EHomingCapability::WireGuided2:
			CurrentWeapon.VehicleWeaponState.BaseWeaponRuntimeData.WeaponState.InFlightProjectiles.RemoveAll([](const TWeakObjectPtr<AProjectile_Base>& Projectile)
			{
				return !Projectile.IsValid() || Projectile->IsHidden();
			});
			for (TWeakObjectPtr<AProjectile_Base> InFlightProjectile : CurrentWeapon.VehicleWeaponState.BaseWeaponRuntimeData.WeaponState.InFlightProjectiles)
			{
				FVector TargetLocation = HitResult.bBlockingHit ? HitResult.ImpactPoint : HitResult.TraceEnd;
				InFlightProjectile->UpdateManualHoming(TargetLocation);
			}
			break;
	}
}

#pragma region LockOn

void UVehicleWeaponLogicComponent::HandleLockOn(int32 SeatIndex, int32 WeaponIndex)
{
	FVehicleWeaponSystem_Runtime& SeatWeaponSystem = *VehicleWeaponSystem.Find(SeatIndex);
	FVehicleWeapon_Runtime& CurrentWeapon = GetEquippedWeaponInSeat(SeatIndex);
	const FBaseWeaponData& StaticWeaponData = GetBaseWeaponDataInSlot(SeatIndex, GetCWIForSeat(SeatIndex));
	const FWeaponHomingData& StaticHomingData = StaticWeaponData.WeaponFunctionality.HomingFunctionality;
	FLockOnState& LockOnState = CurrentWeapon.VehicleWeaponState.BaseWeaponRuntimeData.WeaponState.LockOnState;
	FHitResult& HitResult = SeatWeaponSystem.VehicleWeaponSystemState.EquippedWeaponState.RaycastData.RangefinderData;

	bool ValidLockableActor = HitResult.GetActor() && HitResult.GetActor()->GetClass()->ImplementsInterface(ULockOnTarget::StaticClass());
	if (ValidLockableActor)
	{
		bool canLockOn = ILockOnTarget::Execute_GetIfCanLockOn(HitResult.GetActor(), StaticHomingData.CanTarget, StaticHomingData.HomingCapability);
		bool inRange = StaticHomingData.LockOnRange <= 0 || HitResult.Distance <= StaticHomingData.LockOnRange;

		if (canLockOn && inRange)
		{
			//anything from here on can be seen as a "promotion" of lock status 
			switch (LockOnState.CurrentLockStatus)
			{
				case ELockOnState::NotLockingOn:
					StartLockingOn(SeatIndex, CurrentWeapon, StaticHomingData, HitResult);
					break;
				case ELockOnState::IsLockingOn:
				case ELockOnState::IsLockedOn:
					UpdateLockOnIndicator(OwnerDataAccessor->GetVehicleState().SeatStates[SeatIndex].UpdateHUD, HitResult, LockOnState);
					break;
				case ELockOnState::IsLosingLock:
					break;
			}
		}
		else
		{
			DemoteLockOnStatus(SeatIndex, LockOnState);
		}
	}
	else
	{
		DemoteLockOnStatus(SeatIndex, LockOnState);
	}
}

void UVehicleWeaponLogicComponent::StartLockingOn(int32& SeatIndex, FVehicleWeapon_Runtime& CurrentWeapon, const FWeaponHomingData& HomingData, const FHitResult& HitResult)
{
	UE_LOG(LogTemp, Warning, TEXT("[VWLC::StartLockingOn]"));
	FLockOnState& LockOnState = CurrentWeapon.VehicleWeaponState.BaseWeaponRuntimeData.WeaponState.LockOnState;
	FTimerDelegate LockOnDelegate;
	LockOnDelegate.BindUFunction(this, FName("LockOn"), SeatIndex, HomingData, HitResult);
	GetWorld()->GetTimerManager().SetTimer(LockOnState.LockOnTimer, LockOnDelegate, HomingData.AcquireTime, false);	
	LockOnState.AcquiredTargetComp = HitResult.GetActor()->GetRootComponent();
	LockOnState.CurrentLockStatus = ELockOnState::IsLockingOn;
	GetWAC(SeatIndex)->SetTriggerParameter(FName("Event_LockingOn"));
	//interface to acquired target (locking on)
	if (OwnerDataAccessor->GetVehicleState().SeatStates[SeatIndex].UpdateHUD)
	{
		GetHUDSystem()->SpawnLockOnIndicator(HomingData.IndicatorReticle);
	}
}

void UVehicleWeaponLogicComponent::LockOn(int32 SeatIndex, const FWeaponHomingData HomingData, const FHitResult HitResult)
{
	UE_LOG(LogTemp, Warning, TEXT("[VWLC::LockOn]"));
	FVehicleWeapon_Runtime& CurrentWeapon = GetEquippedWeaponInSeat(SeatIndex);
	FWeaponState& WeaponState = CurrentWeapon.VehicleWeaponState.BaseWeaponRuntimeData.WeaponState;
	WeaponState.LockOnState.CurrentLockStatus = ELockOnState::IsLockedOn;
	GetHUDSystem()->UpdateLockOnIndicatorStatus(WeaponState.LockOnState.CurrentLockStatus);
	GetWorld()->GetTimerManager().ClearTimer(WeaponState.LockOnState.LockOnTimer);
	GetWAC(SeatIndex)->SetTriggerParameter(FName("Event_LockOn"));
	//interface to acquired target (locked on)?... do only when fire
	if (!WeaponState.canFire && WeaponState.CurrentAmmoinMag > 0 && HomingData.HomingCapability == EHomingCapability::RequireLockOn)
	{
		WeaponState.canFire = true;		
		UpdateWeaponStatusUI(SeatIndex, WeaponState.canFire);
	}
}

void UVehicleWeaponLogicComponent::StartCancelLockOn(int32& SeatIndex, FLockOnState& LockOnState)
{
	float ElapsedTime = 1.0f;			//<---make actual data 
	if (GetWorld()->GetTimerManager().IsTimerActive(LockOnState.LockOnTimer))
	{
		ElapsedTime = GetWorld()->GetTimerManager().GetTimerElapsed(LockOnState.LockOnTimer);
	}
	FTimerDelegate LockOnDelegate;
	LockOnDelegate.BindUFunction(this, FName("CancelLockOn"), SeatIndex, GetCWIForSeat(SeatIndex));
	GetWorld()->GetTimerManager().SetTimer(LockOnState.LockOnTimer, LockOnDelegate, ElapsedTime, false);
	LockOnState.CurrentLockStatus = ELockOnState::IsLosingLock;
	GetWAC(SeatIndex)->SetTriggerParameter(FName("Event_StopLockOn"));
	GetHUDSystem()->UpdateLockOnIndicatorStatus(LockOnState.CurrentLockStatus);
	UE_LOG(LogTemp, Warning, TEXT("[VWLC::IsLosingLock]"));
}

void UVehicleWeaponLogicComponent::CancelLockOn(int32 SeatIndex, int32 WeaponIndex)
{
	FWeaponState& WeaponState = VehicleWeaponSystem.Find(SeatIndex)->Weapons[WeaponIndex].VehicleWeaponState.BaseWeaponRuntimeData.WeaponState;
	const FWeaponHomingData& HomingData = GetBaseWeaponDataInSlot(SeatIndex, WeaponIndex).WeaponFunctionality.HomingFunctionality;
	WeaponState.LockOnState.CurrentLockStatus = ELockOnState::NotLockingOn;
	GetHUDSystem()->UpdateLockOnIndicatorStatus(WeaponState.LockOnState.CurrentLockStatus);
	WeaponState.LockOnState.AcquiredTargetComp = nullptr;
	GetWorld()->GetTimerManager().ClearTimer(WeaponState.LockOnState.LockOnTimer);
	GetWAC(SeatIndex)->SetTriggerParameter(FName("Event_StopLockingOn"));
	if (OwnerDataAccessor->GetVehicleState().SeatStates[SeatIndex].UpdateHUD)
	{
		GetHUDSystem()->RemoveWidget(GetHUDSystem()->LockOnIndicator);
	}
	if (HomingData.HomingCapability == EHomingCapability::RequireLockOn)
	{
		WeaponState.canFire = false;
		UpdateWeaponStatusUI(SeatIndex, WeaponState.canFire);
	}
}

void UVehicleWeaponLogicComponent::DemoteLockOnStatus(int32 SeatIndex, FLockOnState& LockOnState)
{
	//null actor or an actor that cant be locked on to
	//anything from here on can be seen as a "demotion" of lock status
	switch (LockOnState.CurrentLockStatus)
	{
		case ELockOnState::IsLockingOn:
		case ELockOnState::IsLockedOn:
			StartCancelLockOn(SeatIndex, LockOnState);
			//interface to target
			//do something to incrementally lose lock, not outright cancel it
			//when completely lost lock, null target, set lock state to notlockingon
			break;
	}
}

void UVehicleWeaponLogicComponent::UpdateLockOnIndicator(bool UpdateHUD, FHitResult& HitResult, FLockOnState& LockOnState)
{
	if (HitResult.GetActor()->GetRootComponent() != LockOnState.AcquiredTargetComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("[VWLC::UpdateSeatRangefinder] DifferentTarget"));
	}
	if (UpdateHUD)
	{
		GetHUDSystem()->UpdateLockOnIndicatorPosition(HitResult.GetActor()->GetRootComponent()->GetSocketLocation(FName("LockOn")));
	}
}

#pragma endregion

void UVehicleWeaponLogicComponent::UpdateManualGuidance(TWeakObjectPtr<AProjectile_Base> FiredProjectile, int32 SeatIndex, int32 WeaponIndex)
{
	//remember, called on tick by rangefinder
	FVehicleWeaponSystem_Runtime& SeatWeaponSystem = *VehicleWeaponSystem.Find(SeatIndex);
	FVehicleWeapon_Runtime& VehicleWeapon = SeatWeaponSystem.Weapons[WeaponIndex];
	FVehicleWeaponState& VehicleWeaponState = VehicleWeapon.VehicleWeaponState;
	FHitResult& HitResult = SeatWeaponSystem.VehicleWeaponSystemState.EquippedWeaponState.RaycastData.RangefinderData;
	const FBaseWeaponData& StaticWeaponData = GetBaseWeaponDataInSlot(SeatIndex, WeaponIndex);

	switch (StaticWeaponData.WeaponFunctionality.HomingFunctionality.HomingCapability)
	{
		case EHomingCapability::WireGuided1:
		case EHomingCapability::WireGuided2:
			FVector TargetLocation = HitResult.bBlockingHit ? HitResult.ImpactPoint : HitResult.TraceEnd;
			FiredProjectile->UpdateManualHoming(TargetLocation);
			break;
	}
}

#pragma endregion

#pragma region Muzzles

FName UVehicleWeaponLogicComponent::BuildMuzzleName(FName SeatName, int32 WeaponIndex, EMuzzleType MuzzleType, int32 MuzzleIndex)
{
	//MS_SeatName_WIName_MuzzleType_MuzzleIndex
	const UEnum* EnumPtr = StaticEnum<EMuzzleType>();
	FString TypeString = EnumPtr ? EnumPtr->GetDisplayNameTextByValue((int64)MuzzleType).ToString() : TEXT("Unknown");
	FString WeaponSlotString = (WeaponIndex == 0) ? TEXT("Primary") : TEXT("Secondary");
	FString FormattedString = FString::Printf(TEXT("MS_%s_%s_%s_%02d"), *SeatName.ToString(), *WeaponSlotString, *TypeString, MuzzleIndex);
	FName FinalSocketName = FName(*FormattedString);
	return FinalSocketName;
}

void UVehicleWeaponLogicComponent::SetupMuzzleSockets(TWeakObjectPtr<USkeletalMeshComponent> Mesh, int32 SeatIndex, int32 WeaponIndex, FName SeatName)
{
	FVehicleWeaponSystem_Runtime* WeaponSystem = VehicleWeaponSystem.Find(SeatIndex);
	FVehicleWeapon_Runtime& VehicleWeaponToFill = WeaponSystem->Weapons[WeaponIndex];
	const FBaseWeaponData& StaticWeaponData = GetBaseWeaponDataInSlot(SeatIndex, WeaponIndex);
	VehicleWeaponToFill.VehicleWeaponState.MuzzleSockets = GetMuzzleSocketsInOrder(Mesh, SeatName, WeaponIndex, VehicleWeaponToFill.VehicleWeaponInstanceData.MuzzleType);

	for (const FName& SocketName : VehicleWeaponToFill.VehicleWeaponState.MuzzleSockets)
	{
		//map with socket name?
		TWeakObjectPtr<UNiagaraComponent> NewVFX = NewObject<UNiagaraComponent>(GetOwner());
		NewVFX->SetupAttachment(Mesh.Get(), SocketName);
		NewVFX->RegisterComponent();
		NewVFX->SetAutoActivate(false);
		NewVFX->SetAsset(StaticWeaponData.WeaponFX.MuzzleFlashFX.LoadSynchronous());
		NewVFX->Deactivate();
		NewVFX->SetNiagaraVariableFloat(TEXT("User.RateOfFire"), UWeaponFunctions::GetFireRate(StaticWeaponData.WeaponFirePerformance.RateOfFire));
		VehicleWeaponToFill.VehicleWeaponState.MuzzleVFXPool.Add(NewVFX);
	}

	switch (VehicleWeaponToFill.VehicleWeaponInstanceData.FireMethod)
	{
		case EFireMethod::Default:
		case EFireMethod::Sequential:
			VehicleWeaponToFill.VehicleWeaponState.CurrentMuzzleIndexes.SetNum(1);
			VehicleWeaponToFill.VehicleWeaponState.CurrentMuzzleIndexes[0] = 0;
			break;
		case EFireMethod::Dual:
			VehicleWeaponToFill.VehicleWeaponState.CurrentMuzzleIndexes.SetNum(2);
			VehicleWeaponToFill.VehicleWeaponState.CurrentMuzzleIndexes[0] = 0;
			VehicleWeaponToFill.VehicleWeaponState.CurrentMuzzleIndexes[1] = 1;
			break;
	}
}

TArray<FName> UVehicleWeaponLogicComponent::GetMuzzleSocketsInOrder(TWeakObjectPtr<USkeletalMeshComponent> Mesh, FName SeatName, int32 WeaponIndex, EMuzzleType MuzzleType)
{
	TArray<FName> FoundSockets;
	for (int32 i = 0; i < 32; i++)
	{
		FName TargetName = BuildMuzzleName(SeatName, WeaponIndex, MuzzleType, i);
		if (Mesh->DoesSocketExist(TargetName))
		{
			FoundSockets.Add(TargetName);
		}
		else
		{
			break;
		}
	}
	return FoundSockets;
}

#pragma endregion

#pragma region WeaponFire

#pragma region StartFire
TWeakObjectPtr<AProjectile_Base> UVehicleWeaponLogicComponent::HandleStartFire(int32 SeatIndex)
{
	//UE_LOG(LogTemp, Warning, TEXT("[VWLC::HandleStartFire"));
	FVehicleWeaponSystem_Runtime& SeatWeaponSystem = *VehicleWeaponSystem.Find(SeatIndex);
	FWeapon_Runtime& CurrentWeapon = SeatWeaponSystem.Weapons[SeatWeaponSystem.VehicleWeaponSystemState.EquippedWeaponState.CurrentWeaponIndex].VehicleWeaponState.BaseWeaponRuntimeData;
	const FBaseWeaponData StaticWeaponData = GetBaseWeaponDataInSlot(SeatIndex, SeatWeaponSystem.VehicleWeaponSystemState.EquippedWeaponState.CurrentWeaponIndex);

	if (!CurrentWeapon.WeaponState.canFire) { return nullptr;}

	TWeakObjectPtr<AProjectile_Base> FiredProjectile = nullptr;

	switch (CurrentWeapon.WeaponState.CurrentFireMode)
	{
		case EFireMode::Single:
			if (CurrentWeapon.WeaponState.isFiring == false)		
			{
				FiredProjectile = StartFire(SeatIndex);
			}
			break;
		case EFireMode::Burst:
			break;
		case EFireMode::Auto:
			if (!GetWorld()->GetTimerManager().IsTimerActive(TimerHandle_AutoFire))
			{
				FiredProjectile = StartFire(SeatIndex);

				//autofire
				if (CurrentWeapon.WeaponState.canFire)	//if here so if the first fire changed this state
				{
					float FireRate = UWeaponFunctions::GetFireRate(StaticWeaponData.WeaponFirePerformance.RateOfFire);

					// Create a Weak Lambda
					FTimerDelegate FireDelegate;
					FireDelegate.BindWeakLambda(this, [this, SeatIndex]()
					{
						// This code ONLY runs if 'this' is still a valid, non-null pointer
						this->FireVehicleWeapon(SeatIndex);
					});

					GetWorld()->GetTimerManager().SetTimer(TimerHandle_AutoFire, FireDelegate, FireRate, true);
				}
			}
			break;
	}

	return FiredProjectile;
}

TWeakObjectPtr<AProjectile_Base> UVehicleWeaponLogicComponent::StartFire(int32 SeatIndex)
{
	//fires exactly one time
	//assumes canFire is true
	FVehicleWeaponSystem_Runtime& SeatWeaponSystem = *VehicleWeaponSystem.Find(SeatIndex);
	FWeapon_Runtime& CurrentWeapon = SeatWeaponSystem.Weapons[GetCWIForSeat(SeatIndex)].VehicleWeaponState.BaseWeaponRuntimeData;
	const FBaseWeaponData StaticWeaponData = GetBaseWeaponDataInSlot(SeatIndex, GetCWIForSeat(SeatIndex));
	TWeakObjectPtr<AProjectile_Base> FiredProjectile = nullptr;

	StartWeaponFireAudio(SeatIndex);
		
	CurrentWeapon.WeaponState.isFiring = true;
	FiredProjectile = FireVehicleWeapon(SeatIndex);	//fire weapon immediately AND THEN (if auto/burst) fire rate every shot after

	return FiredProjectile;
}

void UVehicleWeaponLogicComponent::StartWeaponFireAudio(int32 SeatIndex)
{
	FVehicleWeaponSystem_Runtime& SeatWeaponSystem = *VehicleWeaponSystem.Find(SeatIndex);
	UE_LOG(LogTemp, Warning, TEXT("[VWLC::StartFire] Start audio"));
	TWeakObjectPtr<UAudioComponent> WAC = SeatWeaponSystem.VehicleWeaponSystemState.WeaponAudioComponent;
	WAC->SetTriggerParameter(FName("Event_StartFire"));
}

#pragma endregion

TWeakObjectPtr<AProjectile_Base> UVehicleWeaponLogicComponent::FireVehicleWeapon(int32 SeatIndex)
{
	FVehicleWeaponSystem_Runtime& SeatWeaponSystem = *VehicleWeaponSystem.Find(SeatIndex);
	int32& CWI = GetCWIForSeat(SeatIndex);
	FVehicleWeapon_Runtime& VehicleWeapon = SeatWeaponSystem.Weapons[CWI];
	FVehicleWeaponState& VehicleWeaponState = VehicleWeapon.VehicleWeaponState;
	const FBaseWeaponData& StaticWeaponData = GetBaseWeaponDataInSlot(SeatIndex, CWI);
	FWeapon_Runtime& CurrentWeapon = VehicleWeaponState.BaseWeaponRuntimeData;
	TWeakObjectPtr<AProjectile_Base> FiredProjectile = nullptr;
	switch (StaticWeaponData.WeaponFirePerformance.WeaponFireType)
	{
		case EWeaponFireType::SimProjectile:
			HandleShootSimProjectile(VehicleWeaponState, StaticWeaponData, SeatWeaponSystem);
			break;
		case EWeaponFireType::ActorProjectile:
			FiredProjectile = HandleShootProjectileActor(SeatIndex, CWI);
			break;
		case EWeaponFireType::VFX:
			break;
		case EWeaponFireType::Hitscan:
			break;
	}

	ApplyWeaponRecoilJostle(SeatIndex, CWI);

	//update sequential muzzle
	if (!VehicleWeapon.VehicleWeaponInstanceData.bAreProjectilesMounted && VehicleWeapon.VehicleWeaponInstanceData.FireMethod == EFireMethod::Sequential)
	{
		int32& ActiveIndex = VehicleWeaponState.CurrentMuzzleIndexes[0];
		const int32& TotalMuzzles = VehicleWeaponState.MuzzleSockets.Num();
		ActiveIndex = (ActiveIndex + 1) % TotalMuzzles;
	}

	HandleAmmoDepletion(SeatIndex, CWI);

	return FiredProjectile;
}

#pragma region HandleShootProjectiles

void UVehicleWeaponLogicComponent::HandleShootSimProjectile(FVehicleWeaponState& VehicleWeaponState, const FBaseWeaponData& StaticWeaponData, FVehicleWeaponSystem_Runtime& SeatWeaponSystem)
{
	for (int32& MuzzleIndex : VehicleWeaponState.CurrentMuzzleIndexes)
	{
		FVector MuzzleLocation = FVector::ForwardVector;
		MuzzleLocation = GetMuzzleTransform(VehicleWeaponState, SeatWeaponSystem, MuzzleIndex).GetLocation();

		UWeaponFunctions::CreateSimProjectile
		(
			StaticWeaponData.WeaponFirePerformance.MunitionID,
			nullptr,
			MuzzleLocation,
			StaticWeaponData.WeaponFirePerformance.MuzzleVelocity,
			StaticWeaponData.WeaponFirePerformance.GravityScale,
			SeatWeaponSystem.VehicleWeaponSystemState.EquippedWeaponState.RaycastData.MuzzleAimDirections[MuzzleIndex],
			StaticWeaponData.WeaponFirePerformance.WeaponDamageData.BaseDamage,
			StaticWeaponData.WeaponFirePerformance.WeaponDamageData.DamageDropoffCurve,
			UBS2FunctionLibrary::GetProjectileSystem(this)
		);

		TWeakObjectPtr<UNiagaraComponent> VFXComp = VehicleWeaponState.MuzzleVFXPool[MuzzleIndex];
		if (UNiagaraComponent* VFX = VehicleWeaponState.MuzzleVFXPool[MuzzleIndex].Get())
		{
			VFX->Activate(true);
		}
	}
}

TWeakObjectPtr<AProjectile_Base> UVehicleWeaponLogicComponent::HandleShootProjectileActor(int32 SeatIndex, int32 WeaponIndex)
{
	//not called on tick
	FVehicleWeaponSystem_Runtime& SeatWeaponSystem = *VehicleWeaponSystem.Find(SeatIndex);
	FVehicleWeapon_Runtime& VehicleWeapon = SeatWeaponSystem.Weapons[WeaponIndex];
	FVehicleWeaponState& VehicleWeaponState = VehicleWeapon.VehicleWeaponState;
	FLockOnState& LockOnState = VehicleWeaponState.BaseWeaponRuntimeData.WeaponState.LockOnState;
	const FBaseWeaponData& StaticWeaponData = GetBaseWeaponDataInSlot(SeatIndex, WeaponIndex);
	FHitResult& HitResult = SeatWeaponSystem.VehicleWeaponSystemState.EquippedWeaponState.RaycastData.RangefinderData;

	TWeakObjectPtr<AProjectile_Base> FiredProjectile = nullptr;
	if (VehicleWeapon.VehicleWeaponInstanceData.bAreProjectilesMounted && VehicleWeaponState.CurrentMountedProjectiles.Num() > 0)
	{
		FiredProjectile = VehicleWeaponState.CurrentMountedProjectiles[0];

		SetupProjectileGuidance(FiredProjectile, StaticWeaponData.WeaponFunctionality.HomingFunctionality.HomingCapability, LockOnState, HitResult);

		FiredProjectile->FireProjectile(FiredProjectile->GetActorForwardVector());		//doesnt use aim direction if mounted right now
		//call some sort of "drop from rack" function on projectile?
		if (FiredProjectile.IsValid())
		{
			VehicleWeaponState.CurrentMountedProjectiles.RemoveAt(0, EAllowShrinking::Yes);
			VehicleWeapon.VehicleWeaponState.BaseWeaponRuntimeData.WeaponState.InFlightProjectiles.Add(FiredProjectile);
		}
	}
	else
	{
		for (int32& MuzzleIndex : VehicleWeaponState.CurrentMuzzleIndexes)
		{
			//pull from pool/spawn
			FVector& AimDirection = SeatWeaponSystem.VehicleWeaponSystemState.EquippedWeaponState.RaycastData.MuzzleAimDirections[MuzzleIndex];
			FTransform MuzzleTransform;
			FiredProjectile = UBS2FunctionLibrary::GetProjectileSystem(this)->AcquireProjectileFromPool(StaticWeaponData.WeaponFirePerformance.MunitionID);
			SetupProjectileGuidance(FiredProjectile, StaticWeaponData.WeaponFunctionality.HomingFunctionality.HomingCapability, LockOnState, HitResult);
			AActor* FiringVehicle = GetOwner();
			FiredProjectile->MoveIgnoreActorAdd(FiringVehicle);
			UE_LOG(LogTemp, Error, TEXT("[VWLC::HandleShootProjectileActor] Muzzle Index = %d"), MuzzleIndex);
			MuzzleTransform = GetMuzzleTransform(VehicleWeaponState, SeatWeaponSystem, MuzzleIndex);
			FiredProjectile->SetActorTransform(MuzzleTransform);
			FiredProjectile->FireProjectile(AimDirection);
			VehicleWeapon.VehicleWeaponState.BaseWeaponRuntimeData.WeaponState.InFlightProjectiles.Add(FiredProjectile);
		}
	}

	return FiredProjectile;
}

void UVehicleWeaponLogicComponent::SetupProjectileGuidance(TWeakObjectPtr<AProjectile_Base> FiredProjectile, EHomingCapability HomingCapability, FLockOnState& LockOnState, FHitResult& HitResult)
{
	switch (HomingCapability)
	{
		case EHomingCapability::GPSGuidance:
			FVector TargetLocation = HitResult.bBlockingHit ? HitResult.ImpactPoint : HitResult.TraceEnd;
			FiredProjectile.Get()->UpdateHomingPoint(TargetLocation);
			break;
	}
	if (LockOnState.AcquiredTargetComp.IsValid())
	{
		FiredProjectile->ProjectileMovementComponent->HomingTargetComponent = LockOnState.AcquiredTargetComp.Get();
	}
}

#pragma endregion

#pragma endregion

#pragma region Ammo

void UVehicleWeaponLogicComponent::HandleAmmoDepletion(int32 SeatIndex, int32 WeaponIndex)
{
	const FBaseWeaponData& StaticWeaponData = GetBaseWeaponDataInSlot(SeatIndex, WeaponIndex);
	FVehicleWeapon_Runtime& CurrentVehicleWeapon = GetEquippedWeaponInSeat(SeatIndex);
	FWeapon_Runtime& CurrentWeapon = CurrentVehicleWeapon.VehicleWeaponState.BaseWeaponRuntimeData;
	const FVehicleWeaponInstanceData& VWID = GetVWID(SeatIndex, WeaponIndex, CurrentWeapon.WeaponID);

	switch (StaticWeaponData.AmmoData.AmmoDepletionMethod)
	{
		case EAmmoDepletionMethod::Default:
		{
			switch (VWID.FireMethod)
			{
				case EFireMethod::Default:
				case EFireMethod::Sequential:
					UWeaponFunctions::UpdateCurrentAmmoInMag(CurrentWeapon, -1, StaticWeaponData.AmmoData.MagSize);
					break;
				case EFireMethod::Dual:
					UWeaponFunctions::UpdateCurrentAmmoInMag(CurrentWeapon, -2, StaticWeaponData.AmmoData.MagSize);
					break;
			}

			if (OwnerDataAccessor->GetVehicleState().SeatStates[SeatIndex].UpdateHUD)
			{
				GetHUDSystem()->UpdateStatusHUD_CAMCount(CurrentWeapon.WeaponState.CurrentAmmoinMag);
			}

			if (CurrentWeapon.WeaponState.CurrentAmmoinMag == 0)
			{
				StopFire(SeatIndex);
				HandleStartAutoload(SeatIndex);
				UpdateWeaponStatusUI(SeatIndex, CurrentWeapon.WeaponState.canFire);
			}
			break;
		}
		case EAmmoDepletionMethod::Heat:
			break;
	}
}

void UVehicleWeaponLogicComponent::HandleStartAutoload(int32 SeatIndex)
{
	int32& CWI = GetCWIForSeat(SeatIndex);
	const FBaseWeaponData& StaticWeaponData = GetBaseWeaponDataInSlot(SeatIndex, CWI);
	FWeapon_Runtime& CurrentWeapon = GetEquippedWeaponInSeat(SeatIndex).VehicleWeaponState.BaseWeaponRuntimeData;

	CurrentWeapon.WeaponState.isReloading = true;
	if (CurrentWeapon.WeaponState.CurrentReserveAmmo > 0)
	{
		StartAutoload(StaticWeaponData, SeatIndex, CWI);
	}
	else
	{
		//add dynamic that listens for when reserve ammo is refilled
	}
}

void UVehicleWeaponLogicComponent::StartAutoload(const FBaseWeaponData& StaticWeaponData, int32 SeatIndex, int32 WeaponIndex)
{
	FTimerHandle& TimerHandle_Reload = VehicleWeaponSystem.Find(SeatIndex)->Weapons[WeaponIndex].VehicleWeaponState.BaseWeaponRuntimeData.WeaponState.TimerHandle_Reload;
	if (!GetWorld()->GetTimerManager().IsTimerActive(TimerHandle_Reload))
	{
		const float& ReloadSpeed = StaticWeaponData.AmmoData.ReloadSpeed;
		const int32& MagSize = StaticWeaponData.AmmoData.MagSize;

		GetWorld()->GetTimerManager().SetTimer(TimerHandle_Reload, [this, SeatIndex, WeaponIndex, MagSize]()
		{
			AutoloadNewMag(SeatIndex, WeaponIndex, MagSize);
		}, ReloadSpeed, false);
		//set another timer to track previous timers progress for ui purposes
	}
}

void UVehicleWeaponLogicComponent::AutoloadNewMag(int32 SeatIndex, int32 WeaponIndex, int32 MagSize)
{
	FVehicleWeaponSystem_Runtime& SeatWeaponSystem = *VehicleWeaponSystem.Find(SeatIndex);
	FWeapon_Runtime& CurrentWeapon = SeatWeaponSystem.Weapons[WeaponIndex].VehicleWeaponState.BaseWeaponRuntimeData;
	const FBaseWeaponData& StaticWeaponData = GetBaseWeaponDataInSlot(SeatIndex, WeaponIndex);
	int32 NewCAM, NewCRA;
	UWeaponFunctions::CalculateReload(MagSize, CurrentWeapon.WeaponState.CurrentAmmoinMag, CurrentWeapon.WeaponState.CurrentReserveAmmo, NewCAM, NewCRA);

	CurrentWeapon.WeaponState.CurrentAmmoinMag = NewCAM;
	CurrentWeapon.WeaponState.CurrentReserveAmmo = NewCRA;

	if (SeatWeaponSystem.Weapons[WeaponIndex].VehicleWeaponInstanceData.bAreProjectilesMounted)
	{
		MountProjectiles(SeatIndex, WeaponIndex);
	}

	CurrentWeapon.WeaponState.canFire = true;
	CurrentWeapon.WeaponState.isReloading = false;

	//HMD
	if (OwnerDataAccessor->GetVehicleState().SeatStates[SeatIndex].UpdateHUD && CurrentWeapon.WeaponState.isEquipped)
	{
		GetHUDSystem()->UpdateStatusHUD_CAMCount(NewCAM);
		GetHUDSystem()->UpdateStatusHUD_CRACount(NewCRA);
		GetHUDSystem()->UpdateWeaponStatusHUD_Vehicle(CurrentWeapon.WeaponState.canFire);
	}
	//HUD
	if (OwnerDataAccessor->GetVehicleState().SeatStates[SeatIndex].SeatHUDComponent)
	{
		UUW_HUD_Vehicle_Base* VehicleHUD = Cast<UUW_HUD_Vehicle_Base>(OwnerDataAccessor->GetVehicleState().SeatStates[SeatIndex].SeatHUDComponent->GetUserWidgetObject());
		VehicleHUD->UpdateWeaponStatusHUD(GetEquippedWeaponInSeat(SeatIndex).VehicleWeaponState.BaseWeaponRuntimeData.WeaponState.canFire);
	}

	if (CurrentWeapon.WeaponState.CurrentReserveAmmo < StaticWeaponData.AmmoData.MaxReserveAmmo && StaticWeaponData.AmmoData.AutoRefillReserve)
	{
		//refill reserve
	}
}

#pragma endregion

void UVehicleWeaponLogicComponent::ApplyWeaponRecoilJostle(int32 SeatIndex, int32 WeaponIndex)
{
	FVehicleWeaponSystem_Runtime& SeatWeaponSystem = *VehicleWeaponSystem.Find(SeatIndex);
	float RecoilMultiplier = SeatWeaponSystem.Weapons[WeaponIndex].VehicleWeaponInstanceData.RecoilMultiplier;

	//RECOIL
	//vehicle mesh anim fire
	if (OwnerDataAccessor->GetVehicleMesh()->GetAnimInstance()->GetClass()->ImplementsInterface(UAnims::StaticClass()))
	{
		IAnims::Execute_OnFireWeapon_Vehicle(OwnerDataAccessor->GetVehicleMesh()->GetAnimInstance(), SeatIndex, WeaponIndex);
	}

	//jostle 
	//if mesh has simulate physics
	AVehicle_Base& Vehicle = OwnerDataAccessor->GetVehicle();
	UPrimitiveComponent* RootBody = Cast<UPrimitiveComponent>(Vehicle.GetRootComponent());
	float RelativeYaw;
	if (OwnerDataAccessor->GetVehicleData().Seats[SeatIndex].AvailableItems.ControlledTurretIndexes.Num() > 0)
	{
		int32 TurretIndex = OwnerDataAccessor->GetVehicleData().Seats[SeatIndex].AvailableItems.ControlledTurretIndexes[0];
		UE_LOG(LogTemp, Error, TEXT("[VWLC::ApplyWeaponRecoilJostle] TurretIndex = %d"), TurretIndex);
		RelativeYaw = GetTurretWorldYaw(TurretIndex);
	}
	else
	{
		RelativeYaw = Vehicle.GetActorForwardVector().Y;
	}

	float RadianYaw = FMath::DegreesToRadians(RelativeYaw);
	float PitchFactor = FMath::Cos(RadianYaw);
	float RollFactor = FMath::Sin(RadianYaw);
	FVector HullRight = Vehicle.GetActorRightVector();
	FVector HullForward = Vehicle.GetActorForwardVector();
	FVector JostleTorque = (HullRight * -PitchFactor) + (HullForward * -RollFactor);
	RootBody->AddAngularImpulseInDegrees(JostleTorque * RecoilMultiplier, NAME_None, true);
}

#pragma region StopFiring

void UVehicleWeaponLogicComponent::StopFire(int32 SeatIndex)
{
	FVehicleWeaponSystem_Runtime* SeatWeaponSystemPtr = VehicleWeaponSystem.Find(SeatIndex);
	if (!SeatWeaponSystemPtr)
	{
		UE_LOG(LogTemp, Error, TEXT("[VWLC::StopFire] Failed to find WeaponSystem for SeatIndex: %d"), SeatIndex);
		return;
	}
	FVehicleWeaponSystem_Runtime& SeatWeaponSystem = *SeatWeaponSystemPtr;
	UE_LOG(LogTemp, Warning, TEXT("[VWLC::StopFire] CWI = %d"), SeatWeaponSystem.VehicleWeaponSystemState.EquippedWeaponState.CurrentWeaponIndex);
	StopWeaponSlotFire(SeatIndex, SeatWeaponSystem.VehicleWeaponSystemState.EquippedWeaponState.CurrentWeaponIndex);
}

void UVehicleWeaponLogicComponent::StopWeaponSlotFire(int32 SeatIndex, int32 WeaponIndex)
{
	FVehicleWeaponSystem_Runtime& SeatWeaponSystem = *VehicleWeaponSystem.Find(SeatIndex);
	FVehicleWeaponState& VehicleWeaponState = SeatWeaponSystem.Weapons[WeaponIndex].VehicleWeaponState;
	FWeapon_Runtime& CurrentWeapon = VehicleWeaponState.BaseWeaponRuntimeData;
	UE_LOG(LogTemp, Warning, TEXT("[VWLC::StopWeaponSlotFire] WeaponIndex = %d"), WeaponIndex);
	SeatWeaponSystem.VehicleWeaponSystemState.WeaponAudioComponent->SetTriggerParameter(FName("Event_StopFire"));

	GetWorld()->GetTimerManager().SetTimerForNextTick([this, &VehicleWeaponState]()
	{
		for (int V = 0; V < VehicleWeaponState.MuzzleVFXPool.Num(); V++)
		{
			VehicleWeaponState.MuzzleVFXPool[V]->Deactivate();
		}
	});

	if (GetWorld()->GetTimerManager().IsTimerActive(TimerHandle_AutoFire))
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle_AutoFire);
	}
	CurrentWeapon.WeaponState.isFiring = false;		//crucial for delegates/controlling input state, etc
}

#pragma endregion

#pragma region WeaponManagement

bool UVehicleWeaponLogicComponent::SwitchWeapon(int32 SeatIndex)
{
	FVehicleWeaponSystem_Runtime& SeatWeaponSystem = *VehicleWeaponSystem.Find(SeatIndex);
	int32& CWI = SeatWeaponSystem.VehicleWeaponSystemState.EquippedWeaponState.CurrentWeaponIndex;
	int32 PreviousCWI = CWI;
	bool bWasFiring = SeatWeaponSystem.Weapons[PreviousCWI].VehicleWeaponState.BaseWeaponRuntimeData.WeaponState.isFiring;
	CWI = (CWI + 1) % SeatWeaponSystem.Weapons.Num();

	if (PreviousCWI != CWI)
	{
		UnequipWeapon(SeatIndex, PreviousCWI, bWasFiring);
		EquipWeapon(SeatIndex, CWI);
		return true;
	}
	else
	{
		return false;
	}
}

void UVehicleWeaponLogicComponent::EquipWeapon(int32 SeatIndex, int32 WeaponIndex)
{
	//assumes weapon index in array is valid
	FVehicleWeaponSystem_Runtime& SeatWeaponSystem = *VehicleWeaponSystem.Find(SeatIndex);
	SeatWeaponSystem.VehicleWeaponSystemState.EquippedWeaponState.CurrentWeaponIndex = WeaponIndex;
	const FBaseWeaponData& StaticWeaponData = GetBaseWeaponDataInSlot(SeatIndex, WeaponIndex);
	FVehicleWeapon_Runtime& CurrentVehicleWeapon = GetEquippedWeaponInSeat(SeatIndex);
	FWeapon_Runtime& CurrentWeapon = CurrentVehicleWeapon.VehicleWeaponState.BaseWeaponRuntimeData;
	const FVehicleWeaponInstanceData& VWID = GetVWID(SeatIndex, WeaponIndex, CurrentWeapon.WeaponID);

	UpdateWeaponAudioCompData(SeatIndex, WeaponIndex);

	GetWorld()->GetTimerManager().SetTimerForNextTick([this, SeatIndex, WeaponIndex]()
	{
		FVehicleWeaponSystem_Runtime& SWS = *VehicleWeaponSystem.Find(SeatIndex);
		FWeapon_Runtime& NewWeapon = SWS.Weapons[WeaponIndex].VehicleWeaponState.BaseWeaponRuntimeData;
		const FBaseWeaponData& StaticWeaponData = GetBaseWeaponDataInSlot(SeatIndex, WeaponIndex);

		//HUD/HMD
		if (OwnerDataAccessor->GetVehicleState().SeatStates[SeatIndex].UpdateHUD)
		{
			//HMD
			GetHUDSystem()->UpdateStatusHUD_CAMCount(NewWeapon.WeaponState.CurrentAmmoinMag);
			GetHUDSystem()->UpdateStatusHUD_CRACount(NewWeapon.WeaponState.CurrentReserveAmmo);
			GetHUDSystem()->UpdateEquippedWeaponHUD_Vehicle
			(
				GetBaseWeaponDataInSlot(SeatIndex, WeaponIndex).WeaponClassification.WeaponDisplayNameAbrev,
				GetEquippedWeaponInSeat(SeatIndex).VehicleWeaponInstanceData.WeaponUIInstanceData.WeaponReticle,
				GetEquippedWeaponInSeat(SeatIndex).VehicleWeaponInstanceData.WeaponUIInstanceData.ReticleScale,
				GetEquippedWeaponInSeat(SeatIndex).VehicleWeaponState.BaseWeaponRuntimeData.WeaponState.canFire
			);

			//HUD (HUD Componenet)
			if (OwnerDataAccessor->GetVehicleState().SeatStates[SeatIndex].SeatHUDComponent)
			{
				UUW_HUD_Vehicle_Base* VehicleHUD = Cast<UUW_HUD_Vehicle_Base>(OwnerDataAccessor->GetVehicleState().SeatStates[SeatIndex].SeatHUDComponent->GetUserWidgetObject());
				VehicleHUD->UpdateEquippedWeaponHUD
				(
					GetBaseWeaponDataInSlot(SeatIndex, WeaponIndex).WeaponClassification.WeaponDisplayNameAbrev,
					GetEquippedWeaponInSeat(SeatIndex).VehicleWeaponInstanceData.WeaponUIInstanceData.WeaponReticle,
					GetEquippedWeaponInSeat(SeatIndex).VehicleWeaponInstanceData.WeaponUIInstanceData.ReticleScale,
					GetEquippedWeaponInSeat(SeatIndex).VehicleWeaponState.BaseWeaponRuntimeData.WeaponState.canFire
				);
			}

			//separate quad (reticle)
			TWeakObjectPtr<UStaticMeshComponent> Quad = SWS.VehicleWeaponSystemState.ReticleQuad;
			if (Quad.Get())
			{
				UMaterialInterface* CurrentMat = Quad->GetMaterial(0);
				UMaterialInstanceDynamic* DynMat = Cast<UMaterialInstanceDynamic>(CurrentMat);
				DynMat->SetTextureParameterValue(FName("ReticleTexture"), GetEquippedWeaponInSeat(SeatIndex).VehicleWeaponInstanceData.WeaponUIInstanceData.WeaponReticle);
				Quad->SetRelativeScale3D(FVector::OneVector);
				FVector NewScale = Quad->GetRelativeScale3D() * GetEquippedWeaponInSeat(SeatIndex).VehicleWeaponInstanceData.WeaponUIInstanceData.ReticleScale;
				Quad->SetRelativeScale3D(NewScale);
			}
			//whatever other update HMD (like HUD system updates but without the subsystem)
			//how handle that without the hud system switchboard
		}
	});

	SeatWeaponSystem.Weapons[WeaponIndex].VehicleWeaponState.BaseWeaponRuntimeData.WeaponState.isEquipped = true;

	if (StaticWeaponData.WeaponFunctionality.HomingFunctionality.HomingCapability == EHomingCapability::RequireLockOn)
	{
		GetEquippedWeaponInSeat(SeatIndex).VehicleWeaponState.BaseWeaponRuntimeData.WeaponState.canFire = false;
	}

	FVehicleWeaponSystem_Runtime& SWS = *VehicleWeaponSystem.Find(SeatIndex);
	FWeapon_Runtime& NewWeapon = SWS.Weapons[WeaponIndex].VehicleWeaponState.BaseWeaponRuntimeData;
	if (GetVWID(SeatIndex, WeaponIndex, NewWeapon.WeaponID).bHasSpecialCam)
	{
		OwnerDataAccessor->GetVehicle().UpdateSeatActiveCamera(SeatIndex, SWS.Weapons[WeaponIndex].VehicleWeaponState.WeaponTurretCamera->GetCameraComponent());
	}

	int32 MuzzleCount = SeatWeaponSystem.Weapons[WeaponIndex].VehicleWeaponState.MuzzleSockets.Num();
	SeatWeaponSystem.VehicleWeaponSystemState.EquippedWeaponState.RaycastData.MuzzleAimDirections.SetNum(MuzzleCount);
	//equip weapon audio
	//equip weapon animation
}

void UVehicleWeaponLogicComponent::UnequipWeapon(int32& SeatIndex, int32& WeaponIndex, bool& bWasFiring)
{
	FVehicleWeaponSystem_Runtime& SeatWeaponSystem = *VehicleWeaponSystem.Find(SeatIndex);
	FWeaponState& WeaponState = SeatWeaponSystem.Weapons[WeaponIndex].VehicleWeaponState.BaseWeaponRuntimeData.WeaponState;
	if (bWasFiring)
	{
		StopWeaponSlotFire(SeatIndex, WeaponIndex);
	}
	SeatWeaponSystem.Weapons[WeaponIndex].VehicleWeaponState.BaseWeaponRuntimeData.WeaponState.isEquipped = false;
	if (SeatWeaponSystem.Weapons[WeaponIndex].VehicleWeaponState.BaseWeaponRuntimeData.WeaponState.LockOnState.CurrentLockStatus != ELockOnState::NotLockingOn)
	{
		CancelLockOn(SeatIndex, WeaponIndex);
	}
}

void UVehicleWeaponLogicComponent::UpdateWeaponAudioCompData(int32 SeatIndex, int32 WeaponIndex)
{
	FVehicleWeaponSystem_Runtime& SeatWeaponSystem = *VehicleWeaponSystem.Find(SeatIndex);
	const FBaseWeaponData& StaticData = GetBaseWeaponDataInSlot(SeatIndex, WeaponIndex);
	TWeakObjectPtr<UAudioComponent> WAC = SeatWeaponSystem.VehicleWeaponSystemState.WeaponAudioComponent;

	WAC->SetFloatParameter(FName("Data_RPM"), StaticData.WeaponFirePerformance.RateOfFire);
	WAC->SetObjectParameter(FName("Data_StopFireAudio"), StaticData.WeaponAudio.FireStop.LoadSynchronous());

	TArray<UObject*> LoadedWaves;
	for (const TSoftObjectPtr<USoundWave>& SoftWave : StaticData.WeaponAudio.FireLoop)
	{
		USoundWave* Wave = SoftWave.LoadSynchronous();
		LoadedWaves.Add(Wave);
	}
	WAC->SetObjectArrayParameter(FName("Data_FireLoopAudio"), LoadedWaves);
}

void UVehicleWeaponLogicComponent::UpdateWeaponStatusUI(int32& SeatIndex, bool& canFire)
{
	if (!OwnerDataAccessor->GetVehicleState().SeatStates[SeatIndex].UpdateHUD) { return;}
	//HMD
	GetHUDSystem()->UpdateWeaponStatusHUD_Vehicle(canFire);
	//HUD Comp
	if (OwnerDataAccessor->GetVehicleState().SeatStates[SeatIndex].SeatHUDComponent)
	{
		UUW_HUD_Vehicle_Base* VehicleHUD = Cast<UUW_HUD_Vehicle_Base>(OwnerDataAccessor->GetVehicleState().SeatStates[SeatIndex].SeatHUDComponent->GetUserWidgetObject());
		VehicleHUD->UpdateWeaponStatusHUD(canFire);
	}
}

#pragma endregion

#pragma region Getters

FTransform UVehicleWeaponLogicComponent::GetMuzzleTransform(FVehicleWeaponState& VehicleWeaponState, FVehicleWeaponSystem_Runtime& SeatWeaponSystem, int32 MuzzleIndex)
{
	FTransform MuzzleTransform;
	MuzzleTransform.SetLocation(FVector::ForwardVector);
	if (SeatWeaponSystem.VehicleWeaponSystemState.WeaponSystemMesh.IsValid())
	{
		MuzzleTransform = UWeaponFunctions::GetMuzzleTransform(VehicleWeaponState.MuzzleSockets[MuzzleIndex], SeatWeaponSystem.VehicleWeaponSystemState.WeaponSystemMesh);
	}
	else
	{
		MuzzleTransform = UWeaponFunctions::GetMuzzleTransform(VehicleWeaponState.MuzzleSockets[MuzzleIndex], OwnerDataAccessor->GetVehicleMesh());
	}
	return MuzzleTransform;
}

const FBaseWeaponData& UVehicleWeaponLogicComponent::GetBaseWeaponDataInSlot(int32 SeatIndex, int32 WeaponIndex)
{
	TArray<const FBaseWeaponData*>* WeaponArray = CurrentVehicleBaseWeaponData.Find(SeatIndex);
	const FBaseWeaponData& WeaponData = *(*WeaponArray)[WeaponIndex];
	return WeaponData;
}

const FVehicleWeaponInstanceData& UVehicleWeaponLogicComponent::GetVWID(int32 SeatIndex, int32 WeaponIndex, FName WeaponID)
{
	const TMap<FName, FVehicleWeaponInstanceData>& WeaponChoices = OwnerDataAccessor->GetVehicleData().Seats[SeatIndex].AvailableItems.AvailableWeaponSlots[WeaponIndex].WeaponChoices;
	const FVehicleWeaponInstanceData* WeaponDefPtr = WeaponChoices.Find(WeaponID);
	const FVehicleWeaponInstanceData& VehicleWeaponDefinition = *WeaponDefPtr;
	return VehicleWeaponDefinition;
}

TWeakObjectPtr<AActor> UVehicleWeaponLogicComponent::GetCurrentViewTargetAtSeatIndex(int32 SeatIndex)
{
	//current meaning currently equipped weapon (what is the view context of the currently equipped weapon)
	TWeakObjectPtr<AActor> NewViewTarget = nullptr;
	FVehicleWeaponSystem_Runtime& VWS = *VehicleWeaponSystem.Find(SeatIndex);
	FVehicleWeapon_Runtime& Weapon = VWS.Weapons[VWS.VehicleWeaponSystemState.EquippedWeaponState.CurrentWeaponIndex];
	if (Weapon.VehicleWeaponInstanceData.bHasSpecialCam)
	{
		switch (Weapon.VehicleWeaponInstanceData.WeaponCamBehavior.MountMethod)
		{
			case EVehicleWeaponCamMountMethod::VehicleMesh:
			case EVehicleWeaponCamMountMethod::WeaponMesh:
				NewViewTarget = Weapon.VehicleWeaponState.WeaponTurretCamera;
				break;
			case EVehicleWeaponCamMountMethod::MountedProjectile:
				NewViewTarget = Weapon.VehicleWeaponState.CurrentMountedProjectiles[0];
				break;
		}
		return NewViewTarget;
	}
	else
	{
		return GetOwner();
	}
}

float UVehicleWeaponLogicComponent::GetTurretWorldYaw(int32 TurretIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("[VWLC::GetTurretWorldYaw] TurretIndex = %d"), TurretIndex);
	const FTurretState& TurretState = TurretStates[TurretIndex];
	const FTurretData& TurretData = OwnerDataAccessor->GetVehicleData().Turrets[TurretIndex];
	float HullWorldYaw = OwnerDataAccessor->GetVehicle().GetActorRotation().Yaw;

	int32 SI = GetSeatIndexForTurret(TurretIndex);

	UE_LOG(LogTemp, Warning, TEXT("[VWLC::GetTurretWorldYaw] SeatIndex = %d"), SI);

	TWeakObjectPtr<USkeletalMeshComponent> TurretMesh = VehicleWeaponSystem.Find(SI)->VehicleWeaponSystemState.WeaponSystemMesh.Get();
	const bool& ProjectilesMounted = GetVWID(SI, GetCWIForSeat(SI), VehicleWeaponSystem.Find(SI)->Weapons[GetCWIForSeat(SI)].VehicleWeaponState.BaseWeaponRuntimeData.WeaponID).bAreProjectilesMounted;
	if (ProjectilesMounted)
	{
		return TurretState.CurrentTurretRotation;
	}
	else
	{
		FName SocketName = VehicleWeaponSystem.Find(SI)->Weapons[GetCWIForSeat(SI)].VehicleWeaponState.MuzzleSockets[0];
		UE_LOG(LogTemp, Error, TEXT("[VWLC::GetTurretWorldYaw] SocketName = %s"), *SocketName.ToString());
		float TurretWorldYaw;
		if (TurretMesh.IsValid())
		{
			//separate mesh
			TurretWorldYaw = TurretMesh->GetSocketRotation(SocketName).Yaw;
			float FinalRelativeYaw = TurretWorldYaw - HullWorldYaw;
			return FinalRelativeYaw;
		}
		else
		{
			//turret part of vehicle mesh
			TurretWorldYaw = TurretState.CurrentTurretRotation;
			return TurretWorldYaw;
		}
	}

}

int32 UVehicleWeaponLogicComponent::GetSeatIndexForTurret(int32 TurretIndex)
{
	int32 CTI = -1;
	int32 SI = -1;
	for (int32 i = 0; i < OwnerDataAccessor->GetVehicleData().Seats.Num(); i++)
	{
		if (OwnerDataAccessor->GetVehicleData().Seats[i].AvailableItems.ControlledTurretIndexes.Num() > 0)
		{
			CTI = OwnerDataAccessor->GetVehicleData().Seats[i].AvailableItems.ControlledTurretIndexes[0];
			if (CTI == TurretIndex)
			{
				SI = i;
				return SI;
			}
		}
	}
	return SI;
}

int32& UVehicleWeaponLogicComponent::GetCWIForSeat(int32 SeatIndex)
{
	return VehicleWeaponSystem.Find(SeatIndex)->VehicleWeaponSystemState.EquippedWeaponState.CurrentWeaponIndex;
}

FVehicleWeapon_Runtime& UVehicleWeaponLogicComponent::GetEquippedWeaponInSeat(int32 SeatIndex)
{
	return VehicleWeaponSystem.Find(SeatIndex)->Weapons[GetCWIForSeat(SeatIndex)];
}

TWeakObjectPtr<UHUDSubsystem> UVehicleWeaponLogicComponent::GetHUDSystem()
{
	TWeakObjectPtr<ULocalPlayer> LP = GetWorld()->GetFirstLocalPlayerFromController();
	if (LP.Get())
	{
		TWeakObjectPtr<UHUDSubsystem> HUDSub = LP->GetSubsystem<UHUDSubsystem>();
		return HUDSub;
	}
	return nullptr;
}

TWeakObjectPtr<UAudioComponent> UVehicleWeaponLogicComponent::GetWAC(int32& SeatIndex)
{
	FVehicleWeaponSystem_Runtime& SeatWeaponSystem = *VehicleWeaponSystem.Find(SeatIndex);
	TWeakObjectPtr<UAudioComponent> WAC = SeatWeaponSystem.VehicleWeaponSystemState.WeaponAudioComponent;
	return WAC;
}

#pragma endregion
