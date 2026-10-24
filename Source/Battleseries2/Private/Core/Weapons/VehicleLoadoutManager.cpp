#include "Core/Weapons/VehicleLoadoutManager.h"
#include "Save/SaveSubsystem.h"
#include "Save/PlayerSave_Loadout.h"
#include "Character_Base.h"	
#include "Core/Weapons/Projectiles/Projectile_Base.h"
#include "Data/Items/Weapons/Data_Projectile.h"
#include "Data/Items/Weapons/ProjectileTypes.h"
#include "Data/Items/Weapons/Data_VehicleWeapon.h"
#include "Data/Items/Weapons/WeaponDefaults.h"
#include "Data/Vehicles/VehicleDefaults.h"
#include "Data/Data_VehicleAttachments.h"
#include "Data/Data_Optics.h"
#include "Core/Weapons/WeaponFunctions.h"
#include "Core/UI/VehicleHUDs/UW_HUD_Vehicle_Base.h"
#include "Utilities/HUDSubsystem.h"
#include "Utilities/DataManagerSubsystem.h"
#include "Utilities/I_Anims.h"
#include "Utilities/BS2FunctionLibrary.h"
#include "Camera/CameraActor.h"
#include "Components/AudioComponent.h"
#include "NiagaraComponent.h"
#include "AudioParameterControllerInterface.h"
#include "Vehicle_Base.h"
#include "Kismet/GameplayStatics.h"

UVehicleLoadoutManager::UVehicleLoadoutManager()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UVehicleLoadoutManager::BeginPlay()
{
	Super::BeginPlay();
	OwnerDataAccessor = Cast<IVehicleDataAccessor>(GetOwner());
}

void UVehicleLoadoutManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	HandleSeatRangefinders();
}

#pragma region WeaponFunctions
#pragma region WeaponFactory/Initialization
void UVehicleLoadoutManager::Init_VehicleWeaponSystem(TMap<int32, FSavedSeatLoadout> SeatLoadouts)
{
	const FVehicleData& VehicleData = GetOwnerVehicle()->GetVehicleData();
	const FVehicleTypeDefinition& VehicleTypeDefintion = *GetOwnerVehicle()->VehicleTypeDefintion;
	
	for (int32 SeatIndex = 0; SeatIndex < VehicleTypeDefintion.SeatDefinitions.Num(); SeatIndex++)
	{
		if (VehicleTypeDefintion.SeatDefinitions[SeatIndex].SeatRole == ESeatRole::DriverGunner || VehicleTypeDefintion.SeatDefinitions[SeatIndex].SeatRole == ESeatRole::Gunner)
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
				const TArray<FName> DefaultWeaponIDs = VehicleTypeDefintion.SeatDefinitions[SeatIndex].SeatLoadoutOptions.GetDefaultSeatWeapons();
				ApplyWeaponsToSeat(SeatIndex, DefaultWeaponIDs);
			}
		}
	}
	Init_Turrets(VehicleData.Turrets.Num());
}

void UVehicleLoadoutManager::Init_HUDReticleQuad(int32 SeatIndex)
{
	if (!OwnerDataAccessor->GetVehicleData().Seats[SeatIndex].DefaultCharacterContext.SeatHUD)
	{
		return;
	}
	TObjectPtr<UStaticMesh> DefaultPlane = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
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

	FSeatWeaponSystem_Runtime& SWS = *VehicleWeaponSystem.Find(SeatIndex);
	SWS.SeatWeaponSystemState.ReticleQuad = NewQuad;
}

void UVehicleLoadoutManager::Init_WAC(int32 SeatIndex)
{
	FSeatWeaponSystem_Runtime NewSystem;
	TWeakObjectPtr<UAudioComponent> NewAudioComp = UBS2FunctionLibrary::CreateWAC(this, GetOwner(), OwnerDataAccessor->GetMesh());
	NewSystem.SeatWeaponSystemState.WeaponAudioComponent = NewAudioComp;
	VehicleWeaponSystem.Add(SeatIndex, NewSystem);
}

void UVehicleLoadoutManager::Init_Turrets(int32 NumOfTurrets)
{
	TurretStates.SetNum(NumOfTurrets);
}

#pragma endregion

#pragma region ApplyWeapons

void UVehicleLoadoutManager::ApplyVWID(int32 SeatIndex, int32 WeaponIndex, FName WeaponID)
{
	UE_LOG(LogTemp, Error, TEXT("[VWLC::ApplyWeaponInstanceDataAtIndexToSeat] SI = %d, WI = %d, WeaponID = %s"), SeatIndex, WeaponIndex, *WeaponID.ToString());
	const FVehicleTypeSeatDefinition& SeatDefinition = GetOwnerVehicle()->VehicleTypeDefintion->SeatDefinitions[SeatIndex];
	FSeatWeaponSystem_Runtime* WeaponSystem = VehicleWeaponSystem.Find(SeatIndex);
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
		FName SeatName = (*SeatDefinition.SeatName.ToString());
		if (VehicleWeaponToFill.VehicleWeaponInstanceData.bHasSeparateMesh)
		{
			SetupMuzzleSockets(WeaponSystem->SeatWeaponSystemState.WeaponSystemMesh, SeatIndex, WeaponIndex, SeatName);
		}
		else
		{
			SetupMuzzleSockets(OwnerDataAccessor->GetMesh(), SeatIndex, WeaponIndex, SeatName);
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

void UVehicleLoadoutManager::ApplyWeaponAtIndexToSeat(int32 SeatIndex, int32 WeaponIndex, FName WeaponID)
{
	FSeatWeaponSystem_Runtime* WeaponSystem = VehicleWeaponSystem.Find(SeatIndex);
	FVehicleWeapon_Runtime& VehicleWeaponToFill = WeaponSystem->Weapons[WeaponIndex];
	VehicleWeaponToFill.VehicleWeaponInstanceData = GetVWID(SeatIndex, WeaponIndex, WeaponID);
	FWeaponState& DefaultWeaponDataToFill = VehicleWeaponToFill.VehicleWeaponState.BaseWeaponRuntimeData;

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
	DefaultWeaponDataToFill.CurrentFireMode = VehicleWeaponDataToUse.WeaponData.WeaponFunctionality.WeaponFireModeData.DefaultFireMode;
	DefaultWeaponDataToFill.CurrentAmmoinMag = VehicleWeaponDataToUse.WeaponData.AmmoData.MagSize;
	DefaultWeaponDataToFill.CurrentReserveAmmo = VehicleWeaponDataToUse.WeaponData.AmmoData.MaxReserveAmmo;

	ApplyVWID(SeatIndex, WeaponIndex, WeaponID);
}

void UVehicleLoadoutManager::ApplyStaticWeaponData(int32 SeatIndex, int32 WeaponIndex, FName WeaponID)
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

void UVehicleLoadoutManager::HandleApplyWeaponMesh(int32 SeatIndex, int32 WeaponIndex)
{
	FSeatWeaponSystem_Runtime* WeaponSystem = VehicleWeaponSystem.Find(SeatIndex);
	FVehicleWeapon_Runtime& VehicleWeaponToFill = WeaponSystem->Weapons[WeaponIndex];
	if (VehicleWeaponToFill.VehicleWeaponInstanceData.bHasSeparateMesh && WeaponIndex == OwnerDataAccessor->GetVehicleData().Seats[SeatIndex].AvailableItems.WeaponMeshDriverSlotIndex)
	{
		const FVehicleAttachmentData& AttachmentData = *UBS2FunctionLibrary::GetDataSubsystem(this)->GetVehicleAttachmentDataRow(VehicleWeaponToFill.VehicleWeaponInstanceData.AttachmentInstanceData.AttachmentID);
		TWeakObjectPtr<USkeletalMesh> LoadedMesh = AttachmentData.Attachment_SKM.LoadSynchronous();
		UClass* LoadedAnimClass = AttachmentData.Attachment_AnimClass.LoadSynchronous();
		if (!WeaponSystem->SeatWeaponSystemState.WeaponSystemMesh.IsValid())
		{
			// Create it for the first time
			WeaponSystem->SeatWeaponSystemState.WeaponSystemMesh = ApplyWeaponMeshToVehicle(AttachmentData.Attachment_SKM.Get(), SeatIndex, VehicleWeaponToFill.VehicleWeaponInstanceData.AttachmentInstanceData.AttachmentTransform);
		}
		UpdateWeaponAttachment(*WeaponSystem, LoadedMesh, LoadedAnimClass, VehicleWeaponToFill.VehicleWeaponInstanceData.AttachmentInstanceData.AttachmentTransform, VehicleWeaponToFill.VehicleWeaponInstanceData.AttachmentInstanceData.AttachmentID);

		//apply camo
		FName CurrentVehicleCamo = OwnerDataAccessor->GetVehicleState().GenericVehicleState.CurrentCamo;
		if (CurrentVehicleCamo != NAME_None)
		{
			OwnerDataAccessor->GetVehicle().ApplyCamoToAttachment(WeaponSystem->SeatWeaponSystemState.WeaponSystemMesh, WeaponSystem->SeatWeaponSystemState.WSAttachmentID, CurrentVehicleCamo);
		}
	}
	else if (WeaponSystem->SeatWeaponSystemState.WeaponSystemMesh.IsValid())
	{
		// If the driving slot is changed to a weapon with NO mesh, null the existing mesh
		WeaponSystem->SeatWeaponSystemState.WeaponSystemMesh->SetSkeletalMesh(nullptr);
		WeaponSystem->SeatWeaponSystemState.WSAttachmentID = NAME_None;
	}
}

void UVehicleLoadoutManager::MountProjectiles(int32 SeatIndex, int32 WeaponIndex)
{
	TObjectPtr<USkeletalMeshComponent> VehicleMeshComponent = OwnerDataAccessor->GetMesh();
	FVehicleWeapon_Runtime& SeatWeaponToFill = VehicleWeaponSystem.Find(SeatIndex)->Weapons[WeaponIndex];
	FWeaponState& WeaponDataToFill = SeatWeaponToFill.VehicleWeaponState.BaseWeaponRuntimeData;
	const FVehicleWeaponData* VehicleWeaponRow = UBS2FunctionLibrary::GetDataSubsystem(this)->GetVehicleWeaponDataRow(WeaponDataToFill.WeaponID);
	const int32& MagSize = FMath::Min(VehicleWeaponRow->WeaponData.AmmoData.MagSize, WeaponDataToFill.CurrentAmmoinMag);
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
		NewProjectile->SetPreflightContext(VehicleMeshComponent, SocketName);
		//OwnerDataAccessor->GetVehicle().MoveIgnoreActorAdd(NewProjectile.Get());
		SeatWeaponToFill.VehicleWeaponState.CurrentMountedProjectiles.Add(NewProjectile);
		//DONT INITIALIZE, PROJECTILES SHOULD ALREADY BE INITIALIZED BY POOL SUBYSTEM
	}
}

void UVehicleLoadoutManager::ApplyWeaponDecoratives(const TArray<FDecorative>& WeaponDecoratives, FVehicleWeapon_Runtime& RuntimeWeaponData)
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
			SMComp->SetupAttachment(OwnerDataAccessor->GetMesh(), WeaponDecorative.SocketName);
			SMComp->SetRelativeTransform(WeaponDecorative.TransformOffset);
			SMComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Vehicle, ECollisionResponse::ECR_Ignore);
			SMComp->bRenderCustomDepth = true;
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

void UVehicleLoadoutManager::ApplyWeaponsToSeat(int32 SeatIndex, TArray<FName> WeaponIDs)
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

void UVehicleLoadoutManager::ApplySavedWeaponsToSeat(int32 SeatIndex)
{
	//apply SAVED weapons to seat (consider changing the function name)
	const FVehicleData& VehicleData = OwnerDataAccessor->GetVehicleData();
	const FSavedSeatLoadout& SeatLoadoutSave = UBS2FunctionLibrary::GetSaveSubsystem(this)->GetSeatLoadout(VehicleData.Vehicle_Type, SeatIndex);

	ApplyWeaponsToSeat(SeatIndex, SeatLoadoutSave.Weapons);
}

USkeletalMeshComponent* UVehicleLoadoutManager::ApplyWeaponMeshToVehicle(USkeletalMesh* Mesh, int32 SeatIndex, FTransform MeshTransform)
{
	FString SocketNameString = FString::Printf(TEXT("WM_%02d"), SeatIndex);		//WM_SeatIndex	[WM_00]
	FName SocketName = FName(*SocketNameString);

	TObjectPtr<USkeletalMeshComponent> WeaponComp = NewObject<USkeletalMeshComponent>(GetOwner());
	WeaponComp->SetupAttachment(OwnerDataAccessor->GetMesh(), SocketName);
	WeaponComp->bRenderCustomDepth = true;
	WeaponComp->RegisterComponent(); 

	return WeaponComp;
}

void UVehicleLoadoutManager::UpdateWeaponAttachment(FSeatWeaponSystem_Runtime& WeaponSystem, TWeakObjectPtr<USkeletalMesh> LoadedMesh, UClass* LoadedAnimClass, FTransform Transform, FName AttachmentID)
{
	WeaponSystem.SeatWeaponSystemState.WeaponSystemMesh->SetSkeletalMesh(LoadedMesh.Get());
	WeaponSystem.SeatWeaponSystemState.WeaponSystemMesh->SetAnimInstanceClass(LoadedAnimClass);
	WeaponSystem.SeatWeaponSystemState.WeaponSystemMesh->SetRelativeTransform(Transform);
	WeaponSystem.SeatWeaponSystemState.WSAttachmentID = AttachmentID;
}

void UVehicleLoadoutManager::HandleApplyWeaponDecoratives(int32 SeatIndex, int32 WeaponIndex, FName WeaponID)
{
	FSeatWeaponSystem_Runtime* WeaponSystem = VehicleWeaponSystem.Find(SeatIndex);
	FVehicleWeapon_Runtime& VehicleWeaponToFill = WeaponSystem->Weapons[WeaponIndex];
	FVehicleWeapon_Runtime& SeatWeaponToFill = VehicleWeaponSystem.Find(SeatIndex)->Weapons[WeaponIndex];
	if (VehicleWeaponToFill.VehicleWeaponInstanceData.WeaponDecoratives.Num() > 0)
	{
		//clear any that might be there already
		ClearWeaponDecorativesFromSlot(SeatIndex, WeaponIndex);
	}

	ApplyWeaponDecoratives(VehicleWeaponToFill.VehicleWeaponInstanceData.WeaponDecoratives, VehicleWeaponToFill);
}

void UVehicleLoadoutManager::ConfigureWeaponCam(int32 SeatIndex, int32 WeaponIndex, FSeatWeaponSystem_Runtime& WeaponSystem)
{
	//setup/initialization of the special weapon turret cam
	//spring arm?
	TObjectPtr<AActor> OwningActor = GetOwner();
	FString WCNameString = FString::Printf(TEXT("WC_%02d_%02d"), SeatIndex, WeaponIndex);		//WC_SeatIndex_WeaponIndex	[WC_00_00]
	FName WCSocketName = FName(*WCNameString);
	TWeakObjectPtr<USceneComponent> TargetParent = nullptr;

	switch (WeaponSystem.Weapons[WeaponIndex].VehicleWeaponInstanceData.WeaponCamBehavior.MountMethod)
	{
		case EVehicleWeaponCamMountMethod::VehicleMesh:
			TargetParent = OwnerDataAccessor->GetMesh();
			break;
		case EVehicleWeaponCamMountMethod::WeaponMesh:
			TargetParent = WeaponSystem.SeatWeaponSystemState.WeaponSystemMesh;
			break;
		case EVehicleWeaponCamMountMethod::MountedProjectile:
			//uses the first projectile in the list, used for TV Guided missile and sort where there should be a mag size of only 1
			TargetParent = WeaponSystem.Weapons[WeaponIndex].VehicleWeaponState.CurrentMountedProjectiles[0]->ProjectileMeshComponent;
			break;
	}
	
	UCameraComponent* WeaponCamComp = UBS2FunctionLibrary::CreateAndAttachCamera(GetOwner(), TargetParent.Get(), WCSocketName);
	WeaponSystem.Weapons[WeaponIndex].VehicleWeaponState.WeaponTurretCamera = WeaponCamComp;

	/**
	AVehicle_Base& Vehicle = OwnerDataAccessor->GetVehicle();
	if (WeaponIndex == GetCWIForSeat(SeatIndex))		
	{
		//NOT THE DEFAULT CAM, NO SPECIAL WEAPON CAM SHOULD BE THE DEFAULT CAM
		//if weapon index = currentweaponindex, we make this the active cam
		//Vehicle.UpdateSeatActiveCamera(SeatIndex, WeaponCamComp);		

		//Vehicle.UpdateRemoteActiveCamPP(SeatIndex, UBS2FunctionLibrary::GetDataSubsystem(this)->GetOpticDataRow(OwnerDataAccessor->GetVehicleState().SeatStates[SeatIndex].OpticState.CurrentAvailableOptics[OwnerDataAccessor->GetVehicleState().SeatStates[SeatIndex].OpticState.CurrentOpticIndex])->OpticPPSettings, 1.0f, WeaponCamComp);
	}
	**/
}

#pragma endregion

#pragma region ClearWeapons

void UVehicleLoadoutManager::ClearWeaponDecorativesFromSlot(int32 SeatIndex, int32 WeaponIndex)
{
	FSeatWeaponSystem_Runtime* VWS = VehicleWeaponSystem.Find(SeatIndex);
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

void UVehicleLoadoutManager::ClearWeaponSlotFromSeat(int32 SeatIndex, int32 WeaponIndex)
{
	if (!VehicleWeaponSystem.Find(SeatIndex)->Weapons.IsValidIndex(WeaponIndex))
	{
		return;
	}

	FSeatWeaponSystem_Runtime* VWS = VehicleWeaponSystem.Find(SeatIndex);
	FVehicleWeapon_Runtime& WeaponSlotToClear = VWS->Weapons[WeaponIndex];

	//do the same things as ApplyWeaponInstanceDataAtIndexToSeat... but opposite and in reverse
	if (WeaponSlotToClear.VehicleWeaponState.WeaponTurretCamera)
	{
		WeaponSlotToClear.VehicleWeaponState.WeaponTurretCamera->DestroyComponent();
		WeaponSlotToClear.VehicleWeaponState.WeaponTurretCamera = nullptr;
	}

	//get rid of the current weapon mesh
	if (WeaponIndex == OwnerDataAccessor->GetVehicleData().Seats[SeatIndex].AvailableItems.WeaponMeshDriverSlotIndex && VWS->SeatWeaponSystemState.WeaponSystemMesh.IsValid())
	{
		//if any cameras/weapons are dependent on this mesh and equipped in that moment... stuff will break/crash
		//how to make better?
		VWS->SeatWeaponSystemState.WeaponSystemMesh->SetSkeletalMesh(nullptr);
	}

	//ClearMountedProjectiles
	if (WeaponSlotToClear.VehicleWeaponState.CurrentMountedProjectiles.Num() > 0)
	{
		for (int32 i = 0; i < WeaponSlotToClear.VehicleWeaponState.CurrentMountedProjectiles.Num(); i++)
		{
			TWeakObjectPtr<AProjectile_Base> Projectile = WeaponSlotToClear.VehicleWeaponState.CurrentMountedProjectiles[i];
			UBS2FunctionLibrary::GetProjectileSystem(this)->ReturnProjectileToPool(WeaponSlotToClear.VehicleWeaponState.CurrentMountedProjectiles[i]);
			WeaponSlotToClear.VehicleWeaponState.CurrentMountedProjectiles[i]->SetPreflightContext(nullptr, FName());
			WeaponSlotToClear.VehicleWeaponState.CurrentMountedProjectiles[i] = nullptr;
		}
		WeaponSlotToClear.VehicleWeaponState.CurrentMountedProjectiles.Empty();
	}

	ClearWeaponDecorativesFromSlot(SeatIndex, WeaponIndex);

	WeaponSlotToClear = FVehicleWeapon_Runtime();
}

void UVehicleLoadoutManager::ClearWeaponSystemFromSeat(int32 SeatIndex, bool RemoveFromMap)
{

	FSeatWeaponSystem_Runtime* WeaponSystemToClear = VehicleWeaponSystem.Find(SeatIndex);
	if (!WeaponSystemToClear || WeaponSystemToClear->Weapons.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("[VehicleWeaponSystemComponent::ClearWeaponSystemFromSeat] WeaponSystem at seat %d is null"), SeatIndex);
		return;
	}
	for (int32 i = 0; i < WeaponSystemToClear->Weapons.Num(); i++)
	{
		ClearWeaponSlotFromSeat(SeatIndex, i);
	}
	if (WeaponSystemToClear->SeatWeaponSystemState.WeaponSystemMesh.IsValid())
	{
		WeaponSystemToClear->SeatWeaponSystemState.WeaponSystemMesh->SetSkeletalMesh(nullptr);
	}

	WeaponSystemToClear->Weapons.Empty();

	if (RemoveFromMap)
	{
		VehicleWeaponSystem.Remove(SeatIndex);		//CAUSED CUSTOMIZATION SWITCH VEHICLE TYPE BUG SINCE WEN REMOVED IT MAKES THE MAP GARBAGE MEMORY
	}				
}

void UVehicleLoadoutManager::ClearEntireWeaponSystemFromVehicle()
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
float UVehicleLoadoutManager::CalculateTurret(float InputValue, float TurretSpeed, float CurrentTurretValue)
{
	float NewTurretValue = InputValue * TurretSpeed + CurrentTurretValue;
	return NewTurretValue;
}

void UVehicleLoadoutManager::ControlTurret(FVector2D InputValue, int32 SeatIndex)
{
	const FVehicleData& VehicleData = OwnerDataAccessor->GetVehicleData();
	const FVehicleTypeSeatDefinition& VehicleTypeSeatDefinition = GetOwnerVehicle()->VehicleTypeDefintion->SeatDefinitions[SeatIndex];
	if (!VehicleTypeSeatDefinition.ControlledTurretIndexes.IsValidIndex(0))
	{
		return;
	}
	const int32 ControlledTurretIndex = VehicleTypeSeatDefinition.ControlledTurretIndexes[0];
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
	if (PreviousTurretRotation != NewTurretRotation && OwnerDataAccessor->GetVehicleState().SeatStates[SeatIndex].UpdateHUD)
	{
		if (UCameraComponent* ActiveCam = OwnerDataAccessor->GetVehicle().GetRemoteActiveCam(SeatIndex))
		{
			UBS2FunctionLibrary::GetHUDSubsystem(this)->HandleTurretRotationUpdate(ActiveCam->GetComponentRotation().Yaw);
		}
	}
	if (PreviousTurretPitch != NewTurretPitch && OwnerDataAccessor->GetVehicleState().SeatStates[SeatIndex].UpdateHUD)
	{
		UBS2FunctionLibrary::GetHUDSubsystem(this)->HandleTurretPitchUpdate(PitchMin, PitchMax, NewTurretPitch);
	}
}

void UVehicleLoadoutManager::UpdateTurretMesh(int32 SeatIndex, float TurretRotation, float TurretPitch)
{
	//this should only be called if the equipped weapon for the seat is a separate mesh
	int32& CWI = GetCWIForSeat(SeatIndex); 
	if (VehicleWeaponSystem.Find(SeatIndex)->Weapons[CWI].VehicleWeaponInstanceData.bHasSeparateMesh)
	{
		UAnimInstance* AnimInst = VehicleWeaponSystem.Find(SeatIndex)->SeatWeaponSystemState.WeaponSystemMesh->GetAnimInstance();
		if (AnimInst->GetClass()->ImplementsInterface(UAnims::StaticClass()))
		{
			IAnims::Execute_OnUpdateTurret(AnimInst, TurretRotation, TurretPitch);
		}
	}
}

void UVehicleLoadoutManager::UpdateTurretCam(int32 SeatIndex, float TurretRotation, float TurretPitch)
{
	int32& CWI = GetCWIForSeat(SeatIndex);
	FVehicleWeapon_Runtime& CurrentVehicleWeapon = GetEquippedWeaponInSeat(SeatIndex);
	FWeaponState& CurrentWeapon = CurrentVehicleWeapon.VehicleWeaponState.BaseWeaponRuntimeData;
	const FVehicleWeaponInstanceData& VWID = GetVWID(SeatIndex, CWI, CurrentWeapon.WeaponID);
	FSeatWeaponSystem_Runtime& SWS = *VehicleWeaponSystem.Find(SeatIndex);

	//only works for a special weapon cam thats not on an attacment and is mounted to the vehicle mesh
	if (!VWID.bHasSeparateMesh && VWID.bHasSpecialCam && VWID.WeaponCamBehavior.MountMethod == EVehicleWeaponCamMountMethod::VehicleMesh)
	{
		TObjectPtr<UCameraComponent> ActiveCam = OwnerDataAccessor->GetVehicle().GetRemoteActiveCam(SeatIndex);
		ActiveCam->SetRelativeRotation(FRotator(TurretPitch, TurretRotation, 0.0f));
	}
}

#pragma endregion

#pragma region Rangefinder&AimDirection&LineOfConvergence

void UVehicleLoadoutManager::HandleSeatRangefinders()
{
	const FVehicleData& VehicleData = OwnerDataAccessor->GetVehicleData();
	TArray<AActor*> IgnoredActors;
	GetOwner()->GetAttachedActors(IgnoredActors);
	IgnoredActors.Add(GetOwner());
	for (auto& SeatWeaponSystem : VehicleWeaponSystem)	//for each weapon system in the vehicle
	{
		int32& SeatIndex = SeatWeaponSystem.Key;
		
		//HandleUpdateSeatRangefinder
		const FSeatState& CurrentSeatState = OwnerDataAccessor->GetVehicleState().SeatStates[SeatIndex];
		if (!CurrentSeatState.isOccupied)
		{
			continue;
		}

		int32& WeaponIndex = SeatWeaponSystem.Value.SeatWeaponSystemState.EquippedWeaponState.CurrentWeaponIndex;

		//UPDATE LINE TRACE DATA (hit result)
		const FSeatData& SeatData = VehicleData.Seats[SeatIndex];
		if (SeatData.ViewMethod == E_ViewMethod::Remote || SeatWeaponSystem.Value.Weapons[WeaponIndex].VehicleWeaponInstanceData.bHasSpecialCam)		//if remote or specialweapon cam, do rangefinder
		{
			FTransform CamTransform = OwnerDataAccessor->GetVehicle().GetRemoteActiveCam(SeatIndex)->GetComponentTransform();
			UpdateSeatRangefinder(SeatIndex, CamTransform, {IgnoredActors});
		}
		else
		{
			WindowedRangefinder.Broadcast();
		}

		//Handle Calculate Aim Direction
		FHitResult& HitResult = SeatWeaponSystem.Value.SeatWeaponSystemState.EquippedWeaponState.RaycastData.RangefinderData;
		if (!SeatWeaponSystem.Value.Weapons[WeaponIndex].VehicleWeaponInstanceData.bAreProjectilesMounted)
		{
			//GET MUZZLE SOCKET LOCATION, CALCULATE AIM DIRECTION (line from muzzle to whatever HitResult data)
			if (SeatWeaponSystem.Value.Weapons[WeaponIndex].VehicleWeaponInstanceData.bHasSeparateMesh)
			{
				//separate mesh
				CalculateAimDirection(SeatWeaponSystem.Value.SeatWeaponSystemState.WeaponSystemMesh, HitResult, WeaponIndex, SeatIndex);
			}
			else
			{
				//vehicle mesh
				CalculateAimDirection(OwnerDataAccessor->GetMesh(), HitResult, WeaponIndex, SeatIndex);
			}
		}
	}
}

void UVehicleLoadoutManager::UpdateSeatRangefinder(int32 SeatIndex, FTransform TraceTransform, TArray<AActor*> ActorsToIgnore)
{
	const FSeatState& CurrentSeatState = OwnerDataAccessor->GetVehicleState().SeatStates[SeatIndex];
	const FSeatData& SeatData = OwnerDataAccessor->GetVehicleData().Seats[SeatIndex];
	FSeatWeaponSystem_Runtime* SystemPtr = VehicleWeaponSystem.Find(SeatIndex);
	const FBaseWeaponData& StaticWeaponData = GetBaseWeaponDataInSlot(SeatIndex, GetCWIForSeat(SeatIndex));
	FVehicleWeapon_Runtime& CurrentWeapon = GetEquippedWeaponInSeat(SeatIndex);
	FHitResult& HitResult = SystemPtr->SeatWeaponSystemState.EquippedWeaponState.RaycastData.RangefinderData;

	if (StaticWeaponData.WeaponFunctionality.HomingFunctionality.HomingCapability != EHomingCapability::NoHoming)
	{
		HandleHoming(SeatIndex, TraceTransform, ActorsToIgnore);
	}
	else
	{
		bool bHit = UBS2FunctionLibrary::PerformLineTrace(this, TraceTransform, HitResult, ActorsToIgnore, false);
	}

	if (OwnerDataAccessor->GetVehicleState().SeatStates[SeatIndex].UpdateHUD)
	{
		//HMD
		UBS2FunctionLibrary::GetHUDSubsystem(this)->UpdateRangefinderHUD_Vehicle(HitResult.Distance / 100);	//distance in meter

		//HUD
	}
}

void UVehicleLoadoutManager::CalculateAimDirection(TWeakObjectPtr<USkeletalMeshComponent> Mesh, FHitResult HitResult, int32 WeaponIndex, int32 SeatIndex)
{
	FSeatWeaponSystem_Runtime& SeatWeaponSystem = *VehicleWeaponSystem.Find(SeatIndex);
	int32& CWI = GetCWIForSeat(SeatIndex);
	const FVehicleWeaponInstanceData& VWID = GetVWID(SeatIndex, WeaponIndex, GetEquippedWeaponIDInSeat(SeatIndex));
	FVehicleWeapon_Runtime& VehicleWeapon = SeatWeaponSystem.Weapons[CWI];
	if (!VehicleWeapon.VehicleWeaponInstanceData.bAreProjectilesMounted)		//arent we already check this isnt true before calling this function?
	{
		auto& MuzzleSockets = VehicleWeapon.VehicleWeaponState.MuzzleSockets;
		auto& AimDirections = SeatWeaponSystem.SeatWeaponSystemState.EquippedWeaponState.RaycastData.MuzzleAimDirections;

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
			FTransform MuzzleTransform = UBS2FunctionLibrary::GetMuzzleTransform(MuzzleSocketName, Mesh);
			FVector MuzzleLocation = MuzzleTransform.GetLocation();
			
			float MaxInward = VWID.MaxInwardAngle;
			float MaxOutward = VWID.MaxOutwardAngle;
			const FVector MuzzleForward = MuzzleTransform.GetRotation().GetForwardVector();
			
			//CALCULATE AIM DIRECTION
			AimDirections[MI] = UBS2FunctionLibrary::CalculateFinalAimDirection(HitResult, MuzzleTransform, GetOwner(), MaxInward, MaxOutward);
			
			//Debug 
			FVector RawAimDirection = UBS2FunctionLibrary::CalculateRawAimDirection(HitResult, MuzzleLocation);
			float InwardSign = UBS2FunctionLibrary::GetInwardSign(MuzzleLocation, GetOwner());
			
			const FVector InwardLateralDir = GetOwner()->GetActorRightVector() * InwardSign;
			const FVector OutwardLateralDir = -InwardLateralDir;
			const FVector InwardBoundaryDir = (MuzzleForward * FMath::Cos(FMath::DegreesToRadians(MaxInward)) + InwardLateralDir * FMath::Sin(FMath::DegreesToRadians(MaxInward))).GetSafeNormal();
			const FVector OutwardBoundaryDir = (MuzzleForward * FMath::Cos(FMath::DegreesToRadians(MaxOutward)) + OutwardLateralDir * FMath::Sin(FMath::DegreesToRadians(MaxOutward))).GetSafeNormal();
			
			DrawDebugLine(GetWorld(), MuzzleLocation, MuzzleLocation + MuzzleForward * 100000, FColor::White, false, -1.f, 0, 1.0f);       // cone axis
			DrawDebugLine(GetWorld(), MuzzleLocation, MuzzleLocation + InwardBoundaryDir * 100000, FColor::Red, false, -1.f, 0, 1.0f);     // inward limit (toward hull)
			DrawDebugLine(GetWorld(), MuzzleLocation, MuzzleLocation + OutwardBoundaryDir * 100000, FColor::Cyan, false, -1.f, 0, 1.0f);   // outward limit
			DrawDebugLine(GetWorld(), MuzzleLocation, MuzzleLocation + RawAimDirection * 100000, FColor::Yellow, false, -1.f, 0, 1.0f);    // pre-clamp target direction
			DrawDebugLine(GetWorld(), MuzzleLocation, MuzzleLocation + AimDirections[MI] * 100000, FColor::Green, false, -1.f, 0, 2.5f);		//actu shot direction

			// DEBUG: Draw the convergence line
			//UWeaponFunctions::Debug_ProjectilePath(GetWorld(), MuzzleTransform.GetLocation(), HitResult);
		}
	}
	else
	{

	}
}

#pragma endregion

#pragma region Homing&LockOn

void UVehicleLoadoutManager::HandleHoming(int32 SeatIndex, FTransform TraceTransform, TArray<AActor*> ActorsToIgnore)
{
	//remember, this gets called on tick by the rangefinder
	FSeatWeaponSystem_Runtime* SystemPtr = VehicleWeaponSystem.Find(SeatIndex);
	int32& WeaponIndex = GetCWIForSeat(SeatIndex);
	const FBaseWeaponData& StaticWeaponData = GetBaseWeaponDataInSlot(SeatIndex, WeaponIndex);
	const FWeaponHomingData& StaticHomingData = StaticWeaponData.WeaponFunctionality.HomingFunctionality;
	FVehicleWeapon_Runtime& CurrentWeapon = GetEquippedWeaponInSeat(SeatIndex);
	FHitResult& HitResult = SystemPtr->SeatWeaponSystemState.EquippedWeaponState.RaycastData.RangefinderData;
	
	bool bHit = UBS2FunctionLibrary::PerformWeaponSphereTrace(this, TraceTransform, HitResult, ActorsToIgnore, StaticWeaponData.WeaponFunctionality.HomingFunctionality.LockOnRadius, true);

	switch (StaticHomingData.HomingCapability)
	{
		case EHomingCapability::RequireLockOn:
		case EHomingCapability::CanLockOn:
			HandleLockOn(SeatIndex, WeaponIndex);
			break;
		case EHomingCapability::WireGuided1:
		case EHomingCapability::WireGuided2:
			UBS2FunctionLibrary::HandleInFlightManuallyGuidedMunitions(CurrentWeapon.VehicleWeaponState.BaseWeaponRuntimeData, HitResult);
			break;
	}
}

#pragma region LockOn

void UVehicleLoadoutManager::HandleLockOn(int32 SeatIndex, int32 WeaponIndex)
{
	FSeatWeaponSystem_Runtime& SeatWeaponSystem = *VehicleWeaponSystem.Find(SeatIndex);
	FVehicleWeapon_Runtime& CurrentWeapon = GetEquippedWeaponInSeat(SeatIndex);
	const FBaseWeaponData& StaticWeaponData = GetBaseWeaponDataInSlot(SeatIndex, GetCWIForSeat(SeatIndex));
	const FWeaponHomingData& StaticHomingData = StaticWeaponData.WeaponFunctionality.HomingFunctionality;
	FLockOnState& LockOnState = CurrentWeapon.VehicleWeaponState.BaseWeaponRuntimeData.LockOnState;
	FHitResult& HitResult = SeatWeaponSystem.SeatWeaponSystemState.EquippedWeaponState.RaycastData.RangefinderData;
	
	bool canLockOn = UBS2FunctionLibrary::GetCanLockOn_Final(StaticHomingData, HitResult);

	if (canLockOn)
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
				//DO SOMETHING HERE!!!!!!!!!!!!!!!!!!!!!!!
				break;
		}
	}
	else
	{
		DemoteLockOnStatus(SeatIndex, LockOnState);
	}
}

void UVehicleLoadoutManager::StartLockingOn(int32& SeatIndex, FVehicleWeapon_Runtime& CurrentWeapon, const FWeaponHomingData& HomingData, const FHitResult& HitResult)
{
	UE_LOG(LogTemp, Warning, TEXT("[VWLC::StartLockingOn]"));
	FLockOnState& LockOnState = CurrentWeapon.VehicleWeaponState.BaseWeaponRuntimeData.LockOnState;
	
	FTimerDelegate LockOnDelegate;
	LockOnDelegate.BindUFunction(this, FName("LockOn"), SeatIndex, HomingData, HitResult);
	
	UBS2FunctionLibrary::StartLockingOn(this, LockOnDelegate, HomingData.AcquireTime, HitResult.GetActor(), LockOnState, OwnerDataAccessor->GetVehicleState().SeatStates[SeatIndex].UpdateHUD, HomingData.IndicatorReticle);
	
	UBS2FunctionLibrary::HandleStartWAC(GetWAC(SeatIndex), CurrentWeapon.VehicleWeaponState.BaseWeaponRuntimeData);
	GetWAC(SeatIndex)->SetTriggerParameter(FName("Event_LockingOn"));
}

void UVehicleLoadoutManager::LockOn(int32 SeatIndex, const FWeaponHomingData HomingData, const FHitResult HitResult)
{
	UE_LOG(LogTemp, Warning, TEXT("[VWLC::LockOn]"));
	FVehicleWeapon_Runtime& CurrentWeapon = GetEquippedWeaponInSeat(SeatIndex);
	FWeaponState& WeaponState = CurrentWeapon.VehicleWeaponState.BaseWeaponRuntimeData;
	
	UBS2FunctionLibrary::LockOn(this, WeaponState, OwnerDataAccessor->GetVehicleState().SeatStates[SeatIndex].UpdateHUD, HomingData.HomingCapability);
	GetWAC(SeatIndex)->SetTriggerParameter(FName("Event_LockOn"));
	//interface to acquired target (locked on)?... do only when fire
	
	if (!WeaponState.canFire && UBS2FunctionLibrary::GetIfWeaponShouldBeAbleToFire(WeaponState) && HomingData.HomingCapability == EHomingCapability::RequireLockOn)
	{
		//WeaponState.canFire = true;		
		UpdateWeaponStatusUI(SeatIndex, WeaponState.canFire);
	}
}

void UVehicleLoadoutManager::StartCancelLockOn(int32& SeatIndex, FLockOnState& LockOnState)
{
	float ElapsedTime = 1.0f;			//<---make actual data 
	FTimerDelegate LockOnDelegate;
	LockOnDelegate.BindUFunction(this, FName("CancelLockOn"), SeatIndex, GetCWIForSeat(SeatIndex));
	
	UBS2FunctionLibrary::StartCancelLockOn(this, LockOnDelegate, LockOnState, ElapsedTime, OwnerDataAccessor->GetVehicleState().SeatStates[SeatIndex].UpdateHUD);
	GetWAC(SeatIndex)->SetTriggerParameter(FName("Event_StopLockOn"));
	UE_LOG(LogTemp, Warning, TEXT("[VWLC::IsLosingLock]"));
}

void UVehicleLoadoutManager::CancelLockOn(int32 SeatIndex, int32 WeaponIndex)
{
	FWeaponState& WeaponState = VehicleWeaponSystem.Find(SeatIndex)->Weapons[WeaponIndex].VehicleWeaponState.BaseWeaponRuntimeData;
	const FWeaponHomingData& HomingData = GetBaseWeaponDataInSlot(SeatIndex, WeaponIndex).WeaponFunctionality.HomingFunctionality;
	
	UBS2FunctionLibrary::CancelLockOn(this, WeaponState, OwnerDataAccessor->GetVehicleState().SeatStates[SeatIndex].UpdateHUD, HomingData.HomingCapability);
	GetWAC(SeatIndex)->SetTriggerParameter(FName("Event_StopLockingOn"));

	if (HomingData.HomingCapability == EHomingCapability::RequireLockOn)
	{
		//WeaponState.canFire = false;
		UpdateWeaponStatusUI(SeatIndex, WeaponState.canFire);
	}
}

void UVehicleLoadoutManager::DemoteLockOnStatus(int32 SeatIndex, FLockOnState& LockOnState)
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

void UVehicleLoadoutManager::UpdateLockOnIndicator(bool UpdateHUD, FHitResult& HitResult, FLockOnState& LockOnState)
{
	if (HitResult.GetActor()->GetRootComponent() != LockOnState.AcquiredTargetComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("[VWLC::UpdateSeatRangefinder] DifferentTarget"));
	}
	if (UpdateHUD)
	{
		UBS2FunctionLibrary::GetHUDSubsystem(this)->UpdateLockOnIndicatorPosition(HitResult.GetActor()->GetRootComponent()->GetSocketLocation(FName("LockOn")));
	}
}

#pragma endregion

#pragma endregion

#pragma region Muzzles

FName UVehicleLoadoutManager::BuildMuzzleName(FName SeatName, int32 WeaponIndex, EMuzzleType MuzzleType, int32 MuzzleIndex)
{
	//MS_SeatName_WIName_MuzzleType_MuzzleIndex
	const UEnum* EnumPtr = StaticEnum<EMuzzleType>();
	FString TypeString = EnumPtr ? EnumPtr->GetDisplayNameTextByValue((int64)MuzzleType).ToString() : TEXT("Unknown");
	FString WeaponSlotString = (WeaponIndex == 0) ? TEXT("Primary") : TEXT("Secondary");
	FString FormattedString = FString::Printf(TEXT("MS_%s_%s_%s_%02d"), *SeatName.ToString(), *WeaponSlotString, *TypeString, MuzzleIndex);
	FName FinalSocketName = FName(*FormattedString);
	return FinalSocketName;
}

void UVehicleLoadoutManager::SetupMuzzleSockets(TWeakObjectPtr<USkeletalMeshComponent> Mesh, int32 SeatIndex, int32 WeaponIndex, FName SeatName)
{
	FSeatWeaponSystem_Runtime* WeaponSystem = VehicleWeaponSystem.Find(SeatIndex);
	FVehicleWeapon_Runtime& VehicleWeaponToFill = WeaponSystem->Weapons[WeaponIndex];
	const FBaseWeaponData& StaticWeaponData = GetBaseWeaponDataInSlot(SeatIndex, WeaponIndex);
	VehicleWeaponToFill.VehicleWeaponState.MuzzleSockets = GetMuzzleSocketsInOrder(Mesh, SeatName, WeaponIndex, VehicleWeaponToFill.VehicleWeaponInstanceData.MuzzleType);

	for (const FName& SocketName : VehicleWeaponToFill.VehicleWeaponState.MuzzleSockets)
	{
		//map with socket name?
		TWeakObjectPtr<UNiagaraComponent> NewVFX = NewObject<UNiagaraComponent>(GetOwner());
		NewVFX->SetupAttachment(Mesh.Get(), SocketName);
		NewVFX->SetAutoActivate(false);
		NewVFX->SetAsset(StaticWeaponData.WeaponFX.MuzzleFlashFX.LoadSynchronous());
		NewVFX->SetNiagaraVariableFloat(TEXT("User.RateOfFire"), UBS2FunctionLibrary::GetFireRate(StaticWeaponData.WeaponFirePerformance.RateOfFire));
		NewVFX->RegisterComponent();
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

TArray<FName> UVehicleLoadoutManager::GetMuzzleSocketsInOrder(TWeakObjectPtr<USkeletalMeshComponent> Mesh, FName SeatName, int32 WeaponIndex, EMuzzleType MuzzleType)
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
TWeakObjectPtr<AProjectile_Base> UVehicleLoadoutManager::HandleStartFire(int32 SeatIndex)
{
	//UE_LOG(LogTemp, Warning, TEXT("[VWLC::HandleStartFire"));
	FSeatWeaponSystem_Runtime& SeatWeaponSystem = *VehicleWeaponSystem.Find(SeatIndex);
	FWeaponState& CurrentWeapon = SeatWeaponSystem.Weapons[SeatWeaponSystem.SeatWeaponSystemState.EquippedWeaponState.CurrentWeaponIndex].VehicleWeaponState.BaseWeaponRuntimeData;
	const FBaseWeaponData StaticWeaponData = GetBaseWeaponDataInSlot(SeatIndex, SeatWeaponSystem.SeatWeaponSystemState.EquippedWeaponState.CurrentWeaponIndex);

	if (!CurrentWeapon.canFire) { return nullptr;}

	TWeakObjectPtr<AProjectile_Base> FiredProjectile = nullptr;

	switch (CurrentWeapon.CurrentFireMode)
	{
		case EFireMode::Single:
			if (CurrentWeapon.isFiring == false)		
			{
				FiredProjectile = StartFire(SeatIndex);
			}
			break;
		case EFireMode::Burst:
			break;
		case EFireMode::Auto:
			if (!GetWorld()->GetTimerManager().IsTimerActive(CurrentWeapon.TimerHandle_AutoFire))
			{
				FiredProjectile = StartFire(SeatIndex);

				//autofire
				if (CurrentWeapon.canFire)	//if here so if the first fire changed this state
				{
					float FireRate = UBS2FunctionLibrary::GetFireRate(StaticWeaponData.WeaponFirePerformance.RateOfFire);

					// Create a Weak Lambda
					FTimerDelegate FireDelegate;
					FireDelegate.BindWeakLambda(this, [this, SeatIndex]()
					{
						// This code ONLY runs if 'this' is still a valid, non-null pointer
						this->FireVehicleWeapon(SeatIndex);
					});

					GetWorld()->GetTimerManager().SetTimer(CurrentWeapon.TimerHandle_AutoFire, FireDelegate, FireRate, true);
				}
			}
			break;
	}

	return FiredProjectile;
}

TWeakObjectPtr<AProjectile_Base> UVehicleLoadoutManager::StartFire(int32 SeatIndex)
{
	//fires exactly one time
	//assumes canFire is true
	FSeatWeaponSystem_Runtime& SeatWeaponSystem = *VehicleWeaponSystem.Find(SeatIndex);
	FWeaponState& CurrentWeapon = SeatWeaponSystem.Weapons[GetCWIForSeat(SeatIndex)].VehicleWeaponState.BaseWeaponRuntimeData;
	const FBaseWeaponData StaticWeaponData = GetBaseWeaponDataInSlot(SeatIndex, GetCWIForSeat(SeatIndex));
	TWeakObjectPtr<AProjectile_Base> FiredProjectile = nullptr;
	
	UBS2FunctionLibrary::HandleStartWAC(GetWAC(SeatIndex), CurrentWeapon);
	GetWAC(SeatIndex).Get()->SetTriggerParameter(FName("Event_StartFire"));
		
	CurrentWeapon.isFiring = true;
	FiredProjectile = FireVehicleWeapon(SeatIndex);	//fire weapon immediately AND THEN (if auto/burst) fire rate every shot after

	return FiredProjectile;
}

#pragma endregion

TWeakObjectPtr<AProjectile_Base> UVehicleLoadoutManager::FireVehicleWeapon(int32 SeatIndex)
{
	FSeatWeaponSystem_Runtime& SeatWeaponSystem = *VehicleWeaponSystem.Find(SeatIndex);
	int32& CWI = GetCWIForSeat(SeatIndex);
	FVehicleWeapon_Runtime& VehicleWeapon = SeatWeaponSystem.Weapons[CWI];
	FVehicleWeaponState& VehicleWeaponState = VehicleWeapon.VehicleWeaponState;
	const FBaseWeaponData& StaticWeaponData = GetBaseWeaponDataInSlot(SeatIndex, CWI);
	FWeaponState& CurrentWeapon = VehicleWeaponState.BaseWeaponRuntimeData;
	TWeakObjectPtr<AProjectile_Base> FiredProjectile = nullptr;
	switch (StaticWeaponData.WeaponFirePerformance.WeaponFireType)
	{
		case EWeaponFireType::Munition:
		{
			EMunitionType MunitionType = UBS2FunctionLibrary::GetDataSubsystem(this)->GetProjectileDataRow(StaticWeaponData.WeaponFirePerformance.MunitionID)->MunitionType;
			switch (MunitionType)
			{
				case EMunitionType::SimProjectile:
					HandleShootSimProjectile(VehicleWeaponState, StaticWeaponData, SeatWeaponSystem);
					break;
				case EMunitionType::ActorProjectile:
					FiredProjectile = HandleShootProjectileActor(SeatIndex, CWI);
					break;
			}
			break;
		}
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
	
	for (int32& MuzzleIndex : VehicleWeaponState.CurrentMuzzleIndexes)
	{
		TWeakObjectPtr<UNiagaraComponent> VFXComp = VehicleWeaponState.MuzzleVFXPool[MuzzleIndex];
		if (UNiagaraComponent* VFX = VehicleWeaponState.MuzzleVFXPool[MuzzleIndex].Get())
		{
			VFX->Activate(true);
		}
	}

	HandleAmmoDepletion(SeatIndex, CWI);

	return FiredProjectile;
}

#pragma region HandleShootProjectiles

void UVehicleLoadoutManager::HandleShootSimProjectile(FVehicleWeaponState& VehicleWeaponState, const FBaseWeaponData& StaticWeaponData, FSeatWeaponSystem_Runtime& SeatWeaponSystem)
{
	for (int32& MuzzleIndex : VehicleWeaponState.CurrentMuzzleIndexes)
	{
		FVector MuzzleLocation = FVector::ForwardVector;
		MuzzleLocation = GetMuzzleTransform(VehicleWeaponState, SeatWeaponSystem, MuzzleIndex).GetLocation();

		UBS2FunctionLibrary::CreateSimProjectile
		(
			StaticWeaponData.WeaponFirePerformance.MunitionID,
			nullptr, 
			{ GetOwner()},
			MuzzleLocation,
			StaticWeaponData.WeaponFirePerformance.MuzzleVelocity,
			StaticWeaponData.WeaponFirePerformance.GravityScale,
			SeatWeaponSystem.SeatWeaponSystemState.EquippedWeaponState.RaycastData.MuzzleAimDirections[MuzzleIndex],
			UBS2FunctionLibrary::GetProjectileSystem(this)
		);
	}
}

TWeakObjectPtr<AProjectile_Base> UVehicleLoadoutManager::HandleShootProjectileActor(int32 SeatIndex, int32 WeaponIndex)
{
	//not called on tick
	FSeatWeaponSystem_Runtime& SeatWeaponSystem = *VehicleWeaponSystem.Find(SeatIndex);
	FVehicleWeapon_Runtime& VehicleWeapon = SeatWeaponSystem.Weapons[WeaponIndex];
	FVehicleWeaponState& VehicleWeaponState = VehicleWeapon.VehicleWeaponState;
	FLockOnState& LockOnState = VehicleWeaponState.BaseWeaponRuntimeData.LockOnState;
	const FBaseWeaponData& StaticWeaponData = GetBaseWeaponDataInSlot(SeatIndex, WeaponIndex);
	FHitResult& HitResult = SeatWeaponSystem.SeatWeaponSystemState.EquippedWeaponState.RaycastData.RangefinderData;
	AActor* FiringVehicle = GetOwner();
	TWeakObjectPtr<AProjectile_Base> FiredProjectile = nullptr;
	if (VehicleWeapon.VehicleWeaponInstanceData.bAreProjectilesMounted && VehicleWeaponState.CurrentMountedProjectiles.Num() > 0)
	{
		FiredProjectile = VehicleWeaponState.CurrentMountedProjectiles[0];
		FiredProjectile->UpdateCollisionIgnores(GetOwner());
		UBS2FunctionLibrary::SetupMunitionGuidance(FiredProjectile, StaticWeaponData.WeaponFunctionality.HomingFunctionality.HomingCapability, LockOnState, HitResult);

		FiredProjectile->FireProjectile(FiredProjectile->GetActorForwardVector());		//doesnt use aim direction if mounted right now
		//call some sort of "drop from rack" function on projectile?
		if (FiredProjectile.IsValid())
		{
			VehicleWeaponState.CurrentMountedProjectiles.RemoveAt(0, EAllowShrinking::Yes);
			VehicleWeaponState.BaseWeaponRuntimeData.InFlightProjectiles.Add(FiredProjectile);
		}
	}
	else
	{
		for (int32& MuzzleIndex : VehicleWeaponState.CurrentMuzzleIndexes)
		{
			//pull from pool/spawn
			FVector& AimDirection = SeatWeaponSystem.SeatWeaponSystemState.EquippedWeaponState.RaycastData.MuzzleAimDirections[MuzzleIndex];
			FTransform MuzzleTransform = GetMuzzleTransform(VehicleWeaponState, SeatWeaponSystem, MuzzleIndex);
			FiredProjectile = UBS2FunctionLibrary::ShootUnmountedProjectileActor(Cast<APawn>(GetOwner()), {GetOwner()}, AimDirection, MuzzleTransform, StaticWeaponData.WeaponFirePerformance.MunitionID, StaticWeaponData.WeaponFunctionality.HomingFunctionality.HomingCapability, LockOnState, HitResult);
			
			VehicleWeapon.VehicleWeaponState.BaseWeaponRuntimeData.InFlightProjectiles.Add(FiredProjectile);
		}
	}

	return FiredProjectile;
}

#pragma endregion

#pragma endregion

#pragma region Ammo

void UVehicleLoadoutManager::HandleAmmoDepletion(int32 SeatIndex, int32 WeaponIndex)
{
	const FBaseWeaponData& StaticWeaponData = GetBaseWeaponDataInSlot(SeatIndex, WeaponIndex);
	FVehicleWeapon_Runtime& CurrentVehicleWeapon = GetEquippedWeaponInSeat(SeatIndex);
	FWeaponState& CurrentWeapon = CurrentVehicleWeapon.VehicleWeaponState.BaseWeaponRuntimeData;
	const FVehicleWeaponInstanceData& VWID = GetVWID(SeatIndex, WeaponIndex, CurrentWeapon.WeaponID);

	switch (StaticWeaponData.AmmoData.AmmoDepletionMethod)
	{
		case EAmmoDepletionMethod::Default:
		{
			switch (VWID.FireMethod)
			{
				case EFireMethod::Default:
				case EFireMethod::Sequential:
					UBS2FunctionLibrary::UpdateCurrentAmmoInMag(CurrentWeapon, -1, StaticWeaponData.AmmoData.MagSize);
					break;
				case EFireMethod::Dual:
					UBS2FunctionLibrary::UpdateCurrentAmmoInMag(CurrentWeapon, -2, StaticWeaponData.AmmoData.MagSize);
					break;
			}

			if (OwnerDataAccessor->GetVehicleState().SeatStates[SeatIndex].UpdateHUD)
			{
				UBS2FunctionLibrary::GetHUDSubsystem(this)->UpdateStatusHUD_CAMCount(CurrentWeapon.CurrentAmmoinMag);
			}

			if (CurrentWeapon.CurrentAmmoinMag == 0)
			{
				StopFire(SeatIndex);
				HandleStartAutoload(SeatIndex);
				UpdateWeaponStatusUI(SeatIndex, CurrentWeapon.canFire);
			}
			break;
		}
		case EAmmoDepletionMethod::Heat:
			break;
	}
}

void UVehicleLoadoutManager::HandleStartAutoload(int32 SeatIndex)
{
	int32& CWI = GetCWIForSeat(SeatIndex);
	const FBaseWeaponData& StaticWeaponData = GetBaseWeaponDataInSlot(SeatIndex, CWI);
	FWeaponState& CurrentWeapon = GetEquippedWeaponInSeat(SeatIndex).VehicleWeaponState.BaseWeaponRuntimeData;
	
	if (UBS2FunctionLibrary::GetIfWeaponCanReload(CurrentWeapon, false, StaticWeaponData.AmmoData.MagSize))
	{
		StartAutoload(SeatIndex, CWI);
	}
	else
	{
		//add dynamic that listens for when reserve ammo is refilled
	}
}

void UVehicleLoadoutManager::StartAutoload(int32 SeatIndex, int32 WeaponIndex)
{
	FTimerHandle& TimerHandle_Reload = VehicleWeaponSystem.Find(SeatIndex)->Weapons[WeaponIndex].VehicleWeaponState.BaseWeaponRuntimeData.TimerHandle_Reload;
	const FBaseWeaponData& StaticWeaponData = GetBaseWeaponDataInSlot(SeatIndex, WeaponIndex);
	FWeaponState& CurrentWeapon = VehicleWeaponSystem.Find(SeatIndex)->Weapons[WeaponIndex].VehicleWeaponState.BaseWeaponRuntimeData;
	
	if (GetWorld()->GetTimerManager().IsTimerActive(TimerHandle_Reload))	{ return; }
	
	CurrentWeapon.isReloading = true;
	const float& ReloadSpeed = StaticWeaponData.AmmoData.ReloadSpeed;
	const int32& MagSize = StaticWeaponData.AmmoData.MagSize;
	
	if (CurrentWeapon.isEquipped)
	{
		UGameplayStatics::PlaySound2D(this, StaticWeaponData.WeaponAudio.Reload.LoadSynchronous());
	}

	GetWorld()->GetTimerManager().SetTimer(TimerHandle_Reload, [this, SeatIndex, WeaponIndex, MagSize]()
	{
		AutoloadNewMag(SeatIndex, WeaponIndex, MagSize);
	}, ReloadSpeed, false);
	
	if (StaticWeaponData.AmmoData.AutoRefillReserve)
	{
		
	}
		//set another timer to track previous timers progress for ui purposes
}

void UVehicleLoadoutManager::AutoloadNewMag(int32 SeatIndex, int32 WeaponIndex, int32 MagSize)
{
	FSeatWeaponSystem_Runtime& SeatWeaponSystem = *VehicleWeaponSystem.Find(SeatIndex);
	FWeaponState& CurrentWeapon = SeatWeaponSystem.Weapons[WeaponIndex].VehicleWeaponState.BaseWeaponRuntimeData;
	const FBaseWeaponData& StaticWeaponData = GetBaseWeaponDataInSlot(SeatIndex, WeaponIndex);
	int32 NewCAM, NewCRA;
	UBS2FunctionLibrary::CalculateReload(MagSize, CurrentWeapon.CurrentAmmoinMag, CurrentWeapon.CurrentReserveAmmo, NewCAM, NewCRA);

	CurrentWeapon.CurrentAmmoinMag = NewCAM;
	CurrentWeapon.CurrentReserveAmmo = NewCRA;

	if (SeatWeaponSystem.Weapons[WeaponIndex].VehicleWeaponInstanceData.bAreProjectilesMounted)
	{
		MountProjectiles(SeatIndex, WeaponIndex);
	}

	CurrentWeapon.canFire = true;
	CurrentWeapon.isReloading = false;

	//HMD
	if (OwnerDataAccessor->GetVehicleState().SeatStates[SeatIndex].UpdateHUD && CurrentWeapon.isEquipped)
	{
		UBS2FunctionLibrary::GetHUDSubsystem(this)->UpdateStatusHUD_CAMCount(NewCAM);
		UBS2FunctionLibrary::GetHUDSubsystem(this)->UpdateStatusHUD_CRACount(NewCRA);
		UBS2FunctionLibrary::GetHUDSubsystem(this)->UpdateWeaponStatusHUD_Vehicle(CurrentWeapon.canFire);
	}
	//HUD
	if (OwnerDataAccessor->GetVehicleState().SeatStates[SeatIndex].SeatHUDComponent)
	{
		TObjectPtr<UUW_HUD_Vehicle_Base> VehicleHUD = Cast<UUW_HUD_Vehicle_Base>(OwnerDataAccessor->GetVehicleState().SeatStates[SeatIndex].SeatHUDComponent->GetUserWidgetObject());
		VehicleHUD->UpdateWeaponStatusHUD(GetEquippedWeaponInSeat(SeatIndex).VehicleWeaponState.BaseWeaponRuntimeData.canFire);
	}

	if (CurrentWeapon.CurrentReserveAmmo < StaticWeaponData.AmmoData.MaxReserveAmmo && StaticWeaponData.AmmoData.AutoRefillReserve)
	{
		//refill reserve
	}
}

#pragma endregion

void UVehicleLoadoutManager::ApplyWeaponRecoilJostle(int32 SeatIndex, int32 WeaponIndex)
{
	FSeatWeaponSystem_Runtime& SeatWeaponSystem = *VehicleWeaponSystem.Find(SeatIndex);
	float RecoilMultiplier = SeatWeaponSystem.Weapons[WeaponIndex].VehicleWeaponInstanceData.RecoilMultiplier;

	//RECOIL
	//vehicle mesh anim fire
	if (OwnerDataAccessor->GetMesh()->GetAnimInstance()->GetClass()->ImplementsInterface(UAnims::StaticClass()))
	{
		IAnims::Execute_OnFireWeapon_Vehicle(OwnerDataAccessor->GetMesh()->GetAnimInstance(), SeatIndex, WeaponIndex);
	}

	//jostle 
	//if mesh has simulate physics
	AVehicle_Base& Vehicle = *GetOwnerVehicle();
	UPrimitiveComponent* RootBody = Cast<UPrimitiveComponent>(Vehicle.GetRootComponent());
	float RelativeYaw;
	if (Vehicle.VehicleTypeDefintion->SeatDefinitions[SeatIndex].ControlledTurretIndexes.Num() > 0)
	{
		int32 TurretIndex = Vehicle.VehicleTypeDefintion->SeatDefinitions[SeatIndex].ControlledTurretIndexes[0];
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

void UVehicleLoadoutManager::StopFire(int32 SeatIndex)
{
	FSeatWeaponSystem_Runtime* SeatWeaponSystemPtr = VehicleWeaponSystem.Find(SeatIndex);
	if (!SeatWeaponSystemPtr)
	{
		UE_LOG(LogTemp, Error, TEXT("[VWLC::StopFire] Failed to find WeaponSystem for SeatIndex: %d"), SeatIndex);
		return;
	}
	FSeatWeaponSystem_Runtime& SeatWeaponSystem = *SeatWeaponSystemPtr;
	UE_LOG(LogTemp, Warning, TEXT("[VWLC::StopFire] CWI = %d"), SeatWeaponSystem.SeatWeaponSystemState.EquippedWeaponState.CurrentWeaponIndex);
	StopWeaponSlotFire(SeatIndex, SeatWeaponSystem.SeatWeaponSystemState.EquippedWeaponState.CurrentWeaponIndex);
}

void UVehicleLoadoutManager::StopWeaponSlotFire(int32 SeatIndex, int32 WeaponIndex)
{
	FSeatWeaponSystem_Runtime& SeatWeaponSystem = *VehicleWeaponSystem.Find(SeatIndex);
	FVehicleWeaponState& VehicleWeaponState = SeatWeaponSystem.Weapons[WeaponIndex].VehicleWeaponState;
	FWeaponState& CurrentWeapon = VehicleWeaponState.BaseWeaponRuntimeData;
	UE_LOG(LogTemp, Warning, TEXT("[VWLC::StopWeaponSlotFire] WeaponIndex = %d"), WeaponIndex);
	
	SeatWeaponSystem.SeatWeaponSystemState.WeaponAudioComponent->SetTriggerParameter(FName("Event_StopFire"));
	GetWAC(SeatIndex)->OnAudioFinishedNative.AddWeakLambda(this, [this, SeatIndex](UAudioComponent* FinishedComponent)
	{
		FinishedComponent->Deactivate();
	});

	GetWorld()->GetTimerManager().SetTimerForNextTick([this, &VehicleWeaponState]()
	{
		for (int V = 0; V < VehicleWeaponState.MuzzleVFXPool.Num(); V++)
		{
			VehicleWeaponState.MuzzleVFXPool[V]->Deactivate();
		}
	});

	if (GetWorld()->GetTimerManager().IsTimerActive(CurrentWeapon.TimerHandle_AutoFire))
	{
		GetWorld()->GetTimerManager().ClearTimer(CurrentWeapon.TimerHandle_AutoFire);
	}
	CurrentWeapon.isFiring = false;		//crucial for delegates/controlling input state, etc
}

#pragma endregion

#pragma region WeaponManagement

bool UVehicleLoadoutManager::SwitchWeapon(int32 SeatIndex)
{
	FSeatWeaponSystem_Runtime& SeatWeaponSystem = *VehicleWeaponSystem.Find(SeatIndex);
	int32& CWI = SeatWeaponSystem.SeatWeaponSystemState.EquippedWeaponState.CurrentWeaponIndex;
	int32 PreviousCWI = CWI;
	bool bWasFiring = SeatWeaponSystem.Weapons[PreviousCWI].VehicleWeaponState.BaseWeaponRuntimeData.isFiring;
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

void UVehicleLoadoutManager::EquipWeapon(int32 SeatIndex, int32 WeaponIndex)
{
	//assumes weapon index in array is valid
	FSeatWeaponSystem_Runtime& SeatWeaponSystem = *VehicleWeaponSystem.Find(SeatIndex);
	SeatWeaponSystem.SeatWeaponSystemState.EquippedWeaponState.CurrentWeaponIndex = WeaponIndex;
	const FBaseWeaponData& StaticWeaponData = GetBaseWeaponDataInSlot(SeatIndex, WeaponIndex);
	FVehicleWeapon_Runtime& CurrentVehicleWeapon = GetEquippedWeaponInSeat(SeatIndex);
	FWeaponState& CurrentWeapon = CurrentVehicleWeapon.VehicleWeaponState.BaseWeaponRuntimeData;
	//const FVehicleWeaponInstanceData& VWID = GetVWID(SeatIndex, WeaponIndex, CurrentWeapon.WeaponID);

	UpdateSeatWACData(SeatIndex, WeaponIndex);

	GetWorld()->GetTimerManager().SetTimerForNextTick([this, SeatIndex, WeaponIndex]()
	{
		EquipWeapon_UpdateUI(SeatIndex, WeaponIndex);
	});

	SeatWeaponSystem.Weapons[WeaponIndex].VehicleWeaponState.BaseWeaponRuntimeData.isEquipped = true;

	if (StaticWeaponData.WeaponFunctionality.HomingFunctionality.HomingCapability == EHomingCapability::RequireLockOn)
	{
		GetEquippedWeaponInSeat(SeatIndex).VehicleWeaponState.BaseWeaponRuntimeData.canFire = false;
	}

	FSeatWeaponSystem_Runtime& SWS = *VehicleWeaponSystem.Find(SeatIndex);
	FWeaponState& NewWeapon = SWS.Weapons[WeaponIndex].VehicleWeaponState.BaseWeaponRuntimeData;
	if (GetVWID(SeatIndex, WeaponIndex, NewWeapon.WeaponID).bHasSpecialCam)
	{
		OwnerDataAccessor->GetVehicle().UpdateSeatActiveCamera(SeatIndex, SWS.Weapons[WeaponIndex].VehicleWeaponState.WeaponTurretCamera);
	}

	int32 MuzzleCount = SeatWeaponSystem.Weapons[WeaponIndex].VehicleWeaponState.MuzzleSockets.Num();
	SeatWeaponSystem.SeatWeaponSystemState.EquippedWeaponState.RaycastData.MuzzleAimDirections.SetNum(MuzzleCount);
	//equip weapon audio
	//equip weapon animation
}

void UVehicleLoadoutManager::EquipWeapon_UpdateUI(int32 SeatIndex, int32 WeaponIndex)
{
	FSeatWeaponSystem_Runtime& SWS = *VehicleWeaponSystem.Find(SeatIndex);
	FWeaponState& NewWeapon = SWS.Weapons[WeaponIndex].VehicleWeaponState.BaseWeaponRuntimeData;
	//const FBaseWeaponData& StaticWeaponData = GetBaseWeaponDataInSlot(SeatIndex, WeaponIndex);
	//HUD/HMD
	if (!OwnerDataAccessor->GetVehicleState().SeatStates[SeatIndex].UpdateHUD)		{ return; }
	//HMD
	UBS2FunctionLibrary::GetHUDSubsystem(this)->UpdateStatusHUD_CAMCount(NewWeapon.CurrentAmmoinMag);
	UBS2FunctionLibrary::GetHUDSubsystem(this)->UpdateStatusHUD_CRACount(NewWeapon.CurrentReserveAmmo);
	UBS2FunctionLibrary::GetHUDSubsystem(this)->UpdateEquippedWeaponHUD_Vehicle
	(
		GetBaseWeaponDataInSlot(SeatIndex, WeaponIndex).WeaponClassification.WeaponDisplayNameAbrev,
		GetEquippedWeaponInSeat(SeatIndex).VehicleWeaponInstanceData.WeaponUIInstanceData.WeaponReticle,
		GetEquippedWeaponInSeat(SeatIndex).VehicleWeaponInstanceData.WeaponUIInstanceData.ReticleScale,
		GetEquippedWeaponInSeat(SeatIndex).VehicleWeaponState.BaseWeaponRuntimeData.canFire
	);

	//HUD (HUD Component)
	if (OwnerDataAccessor->GetVehicleState().SeatStates[SeatIndex].SeatHUDComponent)
	{
		TObjectPtr<UUW_HUD_Vehicle_Base> VehicleHUD = Cast<UUW_HUD_Vehicle_Base>(OwnerDataAccessor->GetVehicleState().SeatStates[SeatIndex].SeatHUDComponent->GetUserWidgetObject());
		VehicleHUD->UpdateEquippedWeaponHUD
		(
			GetBaseWeaponDataInSlot(SeatIndex, WeaponIndex).WeaponClassification.WeaponDisplayNameAbrev,
			GetEquippedWeaponInSeat(SeatIndex).VehicleWeaponInstanceData.WeaponUIInstanceData.WeaponReticle,
			GetEquippedWeaponInSeat(SeatIndex).VehicleWeaponInstanceData.WeaponUIInstanceData.ReticleScale,
			GetEquippedWeaponInSeat(SeatIndex).VehicleWeaponState.BaseWeaponRuntimeData.canFire
		);
	}

	//separate quad (reticle)
	TWeakObjectPtr<UStaticMeshComponent> Quad = SWS.SeatWeaponSystemState.ReticleQuad;
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

void UVehicleLoadoutManager::UnequipWeapon(int32& SeatIndex, int32& WeaponIndex, bool& bWasFiring)
{
	FSeatWeaponSystem_Runtime& SeatWeaponSystem = *VehicleWeaponSystem.Find(SeatIndex);
	FWeaponState& WeaponState = SeatWeaponSystem.Weapons[WeaponIndex].VehicleWeaponState.BaseWeaponRuntimeData;
	if (bWasFiring)
	{
		StopWeaponSlotFire(SeatIndex, WeaponIndex);
	}
	SeatWeaponSystem.Weapons[WeaponIndex].VehicleWeaponState.BaseWeaponRuntimeData.isEquipped = false;
	if (SeatWeaponSystem.Weapons[WeaponIndex].VehicleWeaponState.BaseWeaponRuntimeData.LockOnState.CurrentLockStatus != ELockOnState::NotLockingOn)
	{
		CancelLockOn(SeatIndex, WeaponIndex);
	}
}

void UVehicleLoadoutManager::UpdateSeatWACData(int32 SeatIndex, int32 WeaponIndex)
{
	FSeatWeaponSystem_Runtime& SeatWeaponSystem = *VehicleWeaponSystem.Find(SeatIndex);
	const FBaseWeaponData& StaticData = GetBaseWeaponDataInSlot(SeatIndex, WeaponIndex);
	TWeakObjectPtr<UAudioComponent> WAC = SeatWeaponSystem.SeatWeaponSystemState.WeaponAudioComponent;
	
	UBS2FunctionLibrary::UpdateWACData(WAC, StaticData.WeaponFirePerformance.RateOfFire, StaticData.WeaponAudio);
}

void UVehicleLoadoutManager::UpdateWeaponStatusUI(int32& SeatIndex, bool& canFire)
{
	if (!OwnerDataAccessor->GetVehicleState().SeatStates[SeatIndex].UpdateHUD) { return;}
	//HMD
	UBS2FunctionLibrary::GetHUDSubsystem(this)->UpdateWeaponStatusHUD_Vehicle(canFire);
	//HUD Comp
	if (OwnerDataAccessor->GetVehicleState().SeatStates[SeatIndex].SeatHUDComponent)
	{
		TObjectPtr<UUW_HUD_Vehicle_Base> VehicleHUD = Cast<UUW_HUD_Vehicle_Base>(OwnerDataAccessor->GetVehicleState().SeatStates[SeatIndex].SeatHUDComponent->GetUserWidgetObject());
		VehicleHUD->UpdateWeaponStatusHUD(canFire);
	}
}

#pragma endregion
#pragma endregion

#pragma region OpticFunctions

void UVehicleLoadoutManager::Init_Optic(int32 SeatIndex)
{
	FVehicleStartingData& VehicleStartingData = GetOwnerVehicle()->VehicleStartingData;
	const FVehicleTypeDefinition* VehicleTypeData = GetOwnerVehicle()->VehicleTypeDefintion;
	
	VehicleOpticSystem.Add(SeatIndex, FVehicleOpticState());
	
	if (VehicleStartingData.StartingVehicleLoadout.SeatLoadout.Find(SeatIndex) && !VehicleStartingData.StartingVehicleLoadout.SeatLoadout.Find(SeatIndex)->Optic.IsNone())
	{
		VehicleOpticSystem.Find(SeatIndex)->CurrentAvailableOptics.Init(VehicleStartingData.StartingVehicleLoadout.SeatLoadout.Find(SeatIndex)->Optic, 1);
	}
	else
	{
		VehicleOpticSystem.Find(SeatIndex)->CurrentAvailableOptics.Init(VehicleTypeData->SeatDefinitions[SeatIndex].SeatLoadoutOptions.AvailableOptics[0], 1);
	}
}

void UVehicleLoadoutManager::ToggleOptic(int32 SeatIndex)
{
	FVehicleOpticState& SeatOpticSystem = *VehicleOpticSystem.Find(SeatIndex);
	int32 POI = SeatOpticSystem.CurrentOpticIndex;
	int32& NOI = SeatOpticSystem.CurrentOpticIndex; 
	UBS2FunctionLibrary::UpdateOpticIndex(SeatOpticSystem.CurrentAvailableOptics.Num(), NOI);
	if (POI == NOI) { return;  }
	
	const FOpticData* CurrentOpticData = UBS2FunctionLibrary::GetDataSubsystem(this)->GetOpticDataRow(SeatOpticSystem.CurrentAvailableOptics[NOI]);
	const FOpticData& PreviousOpticData = *UBS2FunctionLibrary::GetDataSubsystem(this)->GetOpticDataRow(SeatOpticSystem.CurrentAvailableOptics[POI]);
	if (!CurrentOpticData)
	{
		return;
	}
	
	const bool NewOpticHasPP = CurrentOpticData->OpticPPSettings.WeightedBlendables.Array.Num() > 0;
	const bool OldOpticHasPP = PreviousOpticData.OpticPPSettings.WeightedBlendables.Array.Num() > 0;
	bool NewOpticisMagnified = CurrentOpticData->ZoomMagnification != 1.f;
	bool OldOpticisMagnified = PreviousOpticData.ZoomMagnification != 1.f;
	bool EqualMagnification = CurrentOpticData->ZoomMagnification == PreviousOpticData.ZoomMagnification;
	const bool& UpdateHUD = GetOwnerVehicle()->VehicleCurrentState.SeatStates[SeatIndex].UpdateHUD;
	UCameraComponent* Cam = GetOwnerVehicle()->GetRemoteActiveCam(SeatIndex);
	float& CurrentFOV = Cam->FieldOfView;
	FPostProcessSettings& CurrentPPSettings = Cam->PostProcessSettings;
	float& CurrentPPBlendWeight = Cam->PostProcessBlendWeight;
	const float ReticleScale = GetVWID(SeatIndex, GetCWIForSeat(SeatIndex), GetEquippedWeaponInSeat(SeatIndex).VehicleWeaponState.BaseWeaponRuntimeData.WeaponID).WeaponUIInstanceData.ReticleScale;
	
	UBS2FunctionLibrary::HandleUpdateOptic(90.f, CurrentOpticData->ZoomMagnification, CurrentFOV, CurrentOpticData->OpticPPSettings, CurrentPPSettings, CurrentPPBlendWeight);
	
	if (NewOpticHasPP || !EqualMagnification)
	{
		//power on
		UGameplayStatics::PlaySound2D(GetWorld(), CurrentOpticData->PowerOnSound);
	}
	else if (OldOpticHasPP)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), PreviousOpticData.PowerOffSound);
	}
	if (UpdateHUD)
	{
		if (!EqualMagnification)
		{
			//zoom optic
			UBS2FunctionLibrary::GetHUDSubsystem(this)->UpdateOpticMagnificationHUD_Vehicle(CurrentOpticData->ZoomMagnification);
			UBS2FunctionLibrary::GetHUDSubsystem(this)->UpdateWeaponReticleSize_Vehicle(CurrentOpticData->ZoomMagnification * ReticleScale * 0.75f);
		}
		else
		{
			//unzoom optic
			UBS2FunctionLibrary::GetHUDSubsystem(this)->UpdateOpticMagnificationHUD_Vehicle(CurrentOpticData->ZoomMagnification);
			UBS2FunctionLibrary::GetHUDSubsystem(this)->UpdateWeaponReticleSize_Vehicle(ReticleScale);
		}
		if (NewOpticHasPP)
		{
			UBS2FunctionLibrary::GetHUDSubsystem(this)->UpdateVehicleHUD_Color(CurrentOpticData->InverseUIColor);
		}
		else if (OldOpticHasPP)
		{
			UBS2FunctionLibrary::GetHUDSubsystem(this)->UpdateVehicleHUD_Color(FLinearColor(0.0f, 1.0f, 0.036889f, 1.0f));		//HARD CODED FOR NOW, CHANGE TO BE TAKEN FROM HUD/UI SETTINGS
		}
		
		UBS2FunctionLibrary::GetHUDSubsystem(this)->UpdateOpticNameHUD_Vehicle(CurrentOpticData->OpticDisplayNameAbrev);
	}
}

#pragma endregion

#pragma region Getters

FTransform UVehicleLoadoutManager::GetMuzzleTransform(FVehicleWeaponState& VehicleWeaponState, FSeatWeaponSystem_Runtime& SeatWeaponSystem, int32 MuzzleIndex)
{
	FTransform MuzzleTransform;
	MuzzleTransform.SetLocation(FVector::ForwardVector);
	if (SeatWeaponSystem.SeatWeaponSystemState.WeaponSystemMesh.IsValid())
	{
		MuzzleTransform = UBS2FunctionLibrary::GetMuzzleTransform(VehicleWeaponState.MuzzleSockets[MuzzleIndex], SeatWeaponSystem.SeatWeaponSystemState.WeaponSystemMesh);
	}
	else
	{
		MuzzleTransform = UBS2FunctionLibrary::GetMuzzleTransform(VehicleWeaponState.MuzzleSockets[MuzzleIndex], OwnerDataAccessor->GetMesh());
	}
	return MuzzleTransform;
}

const FBaseWeaponData& UVehicleLoadoutManager::GetBaseWeaponDataInSlot(int32 SeatIndex, int32 WeaponIndex)
{
	TArray<const FBaseWeaponData*>* WeaponArray = CurrentVehicleBaseWeaponData.Find(SeatIndex);
	const FBaseWeaponData& WeaponData = *(*WeaponArray)[WeaponIndex];
	return WeaponData;
}

const FVehicleWeaponInstanceData& UVehicleLoadoutManager::GetVWID(int32 SeatIndex, int32 WeaponIndex, FName WeaponID)
{
	const TMap<FName, FVehicleWeaponInstanceData>& WeaponChoices = OwnerDataAccessor->GetVehicleData().Seats[SeatIndex].AvailableItems.AvailableWeaponSlots[WeaponIndex].WeaponChoices;
	const FVehicleWeaponInstanceData* WeaponDefPtr = WeaponChoices.Find(WeaponID);
	const FVehicleWeaponInstanceData& VehicleWeaponDefinition = *WeaponDefPtr;
	return VehicleWeaponDefinition;
}

TWeakObjectPtr<AActor> UVehicleLoadoutManager::GetCurrentViewTargetAtSeatIndex(int32 SeatIndex)
{
	//current meaning currently equipped weapon (what is the view context of the currently equipped weapon)
	TWeakObjectPtr<AActor> NewViewTarget = nullptr;
	FSeatWeaponSystem_Runtime& VWS = *VehicleWeaponSystem.Find(SeatIndex);
	FVehicleWeapon_Runtime& Weapon = VWS.Weapons[GetCWIForSeat(SeatIndex)];
	if (Weapon.VehicleWeaponInstanceData.bHasSpecialCam)
	{
		switch (Weapon.VehicleWeaponInstanceData.WeaponCamBehavior.MountMethod)
		{
			case EVehicleWeaponCamMountMethod::VehicleMesh:
			case EVehicleWeaponCamMountMethod::WeaponMesh:
				NewViewTarget = &OwnerDataAccessor->GetVehicle();
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

float UVehicleLoadoutManager::GetTurretWorldYaw(int32 TurretIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("[VWLC::GetTurretWorldYaw] TurretIndex = %d"), TurretIndex);
	const FTurretState& TurretState = TurretStates[TurretIndex];
	const FTurretData& TurretData = OwnerDataAccessor->GetVehicleData().Turrets[TurretIndex];
	float HullWorldYaw = OwnerDataAccessor->GetVehicle().GetActorRotation().Yaw;

	int32 SI = GetSeatIndexForTurret(TurretIndex);

	UE_LOG(LogTemp, Warning, TEXT("[VWLC::GetTurretWorldYaw] SeatIndex = %d"), SI);

	TWeakObjectPtr<USkeletalMeshComponent> TurretMesh = VehicleWeaponSystem.Find(SI)->SeatWeaponSystemState.WeaponSystemMesh.Get();
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

int32 UVehicleLoadoutManager::GetSeatIndexForTurret(int32 TurretIndex)
{
	int32 CTI = -1;
	int32 SI = -1;
	for (int32 i = 0; i < GetOwnerVehicle()->VehicleTypeDefintion->GetNumSeats(); i++)
	{
		if (GetOwnerVehicle()->VehicleTypeDefintion->SeatDefinitions[i].ControlledTurretIndexes.Num() > 0)
		{
			CTI = GetOwnerVehicle()->VehicleTypeDefintion->SeatDefinitions[i].ControlledTurretIndexes[0];
			if (CTI == TurretIndex)
			{
				SI = i;
				return SI;
			}
		}
	}
	return SI;
}

int32& UVehicleLoadoutManager::GetCWIForSeat(int32 SeatIndex)
{
	return VehicleWeaponSystem.Find(SeatIndex)->SeatWeaponSystemState.EquippedWeaponState.CurrentWeaponIndex;
}

FVehicleWeapon_Runtime& UVehicleLoadoutManager::GetEquippedWeaponInSeat(int32 SeatIndex)
{
	return VehicleWeaponSystem.Find(SeatIndex)->Weapons[GetCWIForSeat(SeatIndex)];
}

UNiagaraComponent* UVehicleLoadoutManager::DebugMuzzleVFXPool(int32 SeatIndex, int32 WeaponIndex)
{
	if (VehicleWeaponSystem.Find(SeatIndex)->Weapons[WeaponIndex].VehicleWeaponState.MuzzleVFXPool.IsValidIndex(0))
	{
		return VehicleWeaponSystem.Find(SeatIndex)->Weapons[WeaponIndex].VehicleWeaponState.MuzzleVFXPool[0].Get();
	}
	return nullptr;
}

TWeakObjectPtr<UAudioComponent> UVehicleLoadoutManager::GetWAC(int32& SeatIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("[VWLC::GetWAC] SeatIndex =  %d"), SeatIndex);
	FSeatWeaponSystem_Runtime& SeatWeaponSystem = *VehicleWeaponSystem.Find(SeatIndex);
	TWeakObjectPtr<UAudioComponent> WAC = SeatWeaponSystem.SeatWeaponSystemState.WeaponAudioComponent;
	return WAC;
}

FName UVehicleLoadoutManager::GetEquippedWeaponIDInSeat(int32 SeatIndex) 
{
	return GetEquippedWeaponInSeat(SeatIndex).VehicleWeaponState.BaseWeaponRuntimeData.WeaponID;
}

AVehicle_Base* UVehicleLoadoutManager::GetOwnerVehicle()
{
	return Cast<AVehicle_Base>(GetOwner());
}

#pragma endregion
