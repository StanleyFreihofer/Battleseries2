
#include "Core/Weapons/WeaponLogicComponent.h"
#include "Core/Weapons/WeaponFunctions.h"
#include "Character_Base.h"
#include "Vehicle_Base.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Core/Weapons/Projectiles/Projectile_Base.h"
#include "Data/Core/CoreTypes.h"
#include "Data/Items/Weapons/ProjectileTypes.h"
#include "Data/Items/Weapons/Data_Weapon.h"
#include "Data/Items/Weapons/Data_InfantryWeapon.h"
#include "Data/Items/Weapons/Data_WeaponAttachments.h"
#include "Data/Items/Weapons/Data_Projectile.h"
#include "Data/Items/Weapons/WeaponDefaults.h"
#include "Data/Data_Optics.h"
#include "Data/Items/Gadgets/Data_Gadget.h"
#include "Save/SaveSubsystem.h"
#include "Utilities/BS2FunctionLibrary.h"

UWeaponLogicComponent::UWeaponLogicComponent()
{

}

void UWeaponLogicComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UWeaponLogicComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

#pragma region Initialization/Factory

void UWeaponLogicComponent::Init_Loadout(TArray<FName> Weapons, TArray<FPlayerLoadoutConfig_Weapon> WeaponLoadouts, TArray<FName> Gadgets)
{
	TArray<FName> WeaponIDs = Weapons;
	TArray<FPlayerLoadoutConfig_Weapon> FinalWeaponLoadouts = WeaponLoadouts;
	TArray<FName> GadgetIDs;
	Loadout.ResolvedGadgetSlots.SetNum(Gadgets.Num());
	
	//INIT_ResolveWeaponGadgets
	//add any gadgets that are weapons to the weaponID list for weapon initialization
	for (int32 i = 0; i < Gadgets.Num(); i++)
	{
		const FGadgetData* NewGadgetData = UBS2FunctionLibrary::GetDataSubsystem(this)->GetGadgetDataRow(Gadgets[i]);
		if (NewGadgetData)
		{
			Loadout.ResolvedGadgetSlots[i].ActualType = ECharacterItemType::Gadget;
			Loadout.ResolvedGadgetSlots[i].ResolvedArrayIndex = GadgetIDs.Num();
			GadgetIDs.Add(Gadgets[i]);
			continue;
		}
		
		//if a data row isn't found given the id, it might actually be a weapon
		//if the gadgetid is in fact a valid infantry weapon data row, remove the id from the gadget list, add it to the weapon list.
		const FInfantryWeaponData* InfantryWeaponData = UBS2FunctionLibrary::GetDataSubsystem(this)->GetInfantryWeaponDataRow(Gadgets[i]);
		if (InfantryWeaponData)
		{
			Loadout.ResolvedGadgetSlots[i].ActualType = ECharacterItemType::Weapon;
			Loadout.ResolvedGadgetSlots[i].ResolvedArrayIndex = WeaponIDs.Num();
			WeaponIDs.Add(Gadgets[i]);
			
			//??? meant to apply weapon attachments in weaponloadout list to weapon gadgets, possible bugs
			if (!FinalWeaponLoadouts.IsValidIndex(i))
			{
				FinalWeaponLoadouts.Add(FPlayerLoadoutConfig_Weapon()); 
			}
		}
	}
	
	Init_WeaponLoadout(WeaponIDs, FinalWeaponLoadouts);
	Init_GadgetLoadout(GadgetIDs);
}

void UWeaponLogicComponent::Init_WeaponLoadout(TArray<FName> Weapons, TArray<FPlayerLoadoutConfig_Weapon> WeaponLoadouts)
{
	Loadout.WeaponSystem.BaseWeaponSystem.Weapons.SetNum(Weapons.Num());
	StaticWeaponDataCache.SetNum(Weapons.Num());
	Loadout.WeaponSystem.InfantryWeaponSystem.WeaponState_FP.SetNum(Weapons.Num());
	Loadout.WeaponSystem.InfantryWeaponSystem.WeaponState_TP.SetNum(Weapons.Num());
	Loadout.WeaponSystem.InfantryWeaponSystem.CurrentWeaponStats.SetNum(Weapons.Num());

	for (int32 i = 0; i < Weapons.Num(); i++)
	{
		Init_Weapon(Weapons[i], i, WeaponLoadouts[i]);
	}
	Init_ScopeCamera();
	EquipWeapon(0, true);
}

void UWeaponLogicComponent::Init_GadgetLoadout(TArray<FName> Gadgets)
{
	Loadout.Gadgets.SetNum(Gadgets.Num());
	StaticGadgetDataCache.SetNum(Gadgets.Num());
	
	for (int32 i = 0; i < Gadgets.Num(); i++)
	{
		Init_Gadget(Gadgets[i], i);
	}
}

void UWeaponLogicComponent::Init_ScopeCamera()
{
	Loadout.WeaponSystem.ScopeCamera = NewObject<USceneCaptureComponent2D>(GetOwner());
	TObjectPtr<USceneCaptureComponent2D>& ScopeCamera = Loadout.WeaponSystem.ScopeCamera;
	ScopeCamera->RegisterComponent();
	ScopeCamera->TextureTarget = UKismetRenderingLibrary::CreateRenderTarget2D(this, 256, 256, RTF_RGBA16f);
	ScopeCamera->HideActorComponents(GetOwner(), true);
	ScopeCamera->bCaptureEveryFrame = true;
	ScopeCamera->CaptureSource = ESceneCaptureSource::SCS_SceneColorHDR;
	UpdateScope(0);
}

void UWeaponLogicComponent::Init_Weapon(FName WeaponID, int32 WeaponIndex, FPlayerLoadoutConfig_Weapon WeaponLoadout)
{
	//Init WeaponSlot		Init Weapon
	FInfantryWeaponState NewFPState;
	Init_WeaponMesh(NewFPState.WeaponMesh);
	NewFPState.WeaponMesh->SetOnlyOwnerSee(true);
	UpdateWeaponMesh(WeaponID, NewFPState.WeaponMesh);
	NewFPState.WeaponMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	Loadout.WeaponSystem.InfantryWeaponSystem.WeaponState_FP[WeaponIndex] = NewFPState;
	
	FString SocketString = FString::Printf(TEXT("Socket_%s"), *WeaponID.ToString());
	FName AttachSocketName = FName(*SocketString);
	Loadout.WeaponSystem.InfantryWeaponSystem.WeaponState_FP[WeaponIndex].WeaponMesh->AttachToComponent(Cast<ACharacter_Base>(GetOwner())->FPArms, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, true), AttachSocketName);
	
	Loadout.WeaponSystem.BaseWeaponSystem.Weapons[WeaponIndex].WeaponID = WeaponID;
	StaticWeaponDataCache[WeaponIndex] = UBS2FunctionLibrary::GetDataSubsystem(this)->GetInfantryWeaponDataRow(WeaponID);
	
	SetupCustomWeapon(WeaponIndex, WeaponLoadout);

	//weaponstate
	FWeaponStats_Runtime& CurrentWeaponStats = Loadout.WeaponSystem.InfantryWeaponSystem.CurrentWeaponStats[WeaponIndex];
	FWeapon_Runtime& BaseWeaponState = GetBaseWeaponState(WeaponIndex);
	BaseWeaponState.WeaponState.CurrentAmmoinMag = CurrentWeaponStats.MagSize;
	BaseWeaponState.WeaponState.CurrentReserveAmmo = CurrentWeaponStats.MaxReserveAmmo;
	BaseWeaponState.WeaponState.CurrentFireMode = CurrentWeaponStats.FireModeData.DefaultFireMode;
	
	UpdateWeaponVisibility(WeaponIndex, true);
}

void UWeaponLogicComponent::Init_WeaponMesh(TWeakObjectPtr<USkeletalMeshComponent>& WeaponMesh)
{
	TWeakObjectPtr<USkeletalMeshComponent> NewWeapon = NewObject<USkeletalMeshComponent>(GetOwner());
	NewWeapon->RegisterComponent();
	WeaponMesh = NewWeapon;			//cache
}

void UWeaponLogicComponent::Init_Attachment(FWeaponAttachmentState& RuntimeSlotState, FInfantryWeaponState& WeaponToApplyTo, EAttachmentSlot AttachmentSlot)
{
	//Initialize/Create Attachment, attach to gun, cache
	TWeakObjectPtr<UStaticMeshComponent> NewAttachment = NewObject<UStaticMeshComponent>(GetOwner());
	NewAttachment->RegisterComponent();
	NewAttachment->AttachToComponent(WeaponToApplyTo.WeaponMesh.Get(), FAttachmentTransformRules::SnapToTargetIncludingScale, GetSocketNameForSlot(AttachmentSlot));
	RuntimeSlotState.SpawnedAttachment = NewAttachment;
}

void UWeaponLogicComponent::Init_Gadget(FName GadgetID, int32 GadgetIndex)
{
	FGadgetState& GadgetState = Loadout.Gadgets[GadgetIndex];
	StaticGadgetDataCache[GadgetIndex] = UBS2FunctionLibrary::GetDataSubsystem(this)->GetGadgetDataRow(GadgetID);
	
	Init_GadgetMesh(GadgetState.HeldMesh_FP);
	GadgetState.HeldMesh_FP->SetOnlyOwnerSee(true);
	UpdateGadgetMesh(GadgetID, GadgetState.HeldMesh_FP);
	
	FString SocketString = FString::Printf(TEXT("Socket_%s"), *GadgetID.ToString());
	FName AttachSocketName = FName(*SocketString);
	Loadout.Gadgets[GadgetIndex].HeldMesh_FP->AttachToComponent(Cast<ACharacter_Base>(GetOwner())->FPArms, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, true), AttachSocketName);
	
	GadgetState.GadgetID = GadgetID;
	GadgetState.CurrentInventory = StaticGadgetDataCache[GadgetIndex]->DefaultInventoryCount;
	
	UpdateGadgetVisibility(GadgetIndex, true);
}

void UWeaponLogicComponent::Init_GadgetMesh(TWeakObjectPtr<UStaticMeshComponent>& HeldGadgetMesh)
{
	TWeakObjectPtr<UStaticMeshComponent> NewGadgetMesh = NewObject<UStaticMeshComponent>(GetOwner());
	NewGadgetMesh->RegisterComponent();
	HeldGadgetMesh = NewGadgetMesh;
}

#pragma endregion

void UWeaponLogicComponent::SetupCustomWeapon(int32 WeaponIndex, FPlayerLoadoutConfig_Weapon WeaponLoadout)
{
	//setup custom weapon (attachments, stats, etc)
	const FPlayerLoadoutConfig_Weapon& CustomWeapon = WeaponLoadout;
	ApplyAttachments(CustomWeapon, WeaponIndex);
	UpdateCurrentWeaponStats(WeaponIndex);
}

void UWeaponLogicComponent::ApplyAttachments(const FPlayerLoadoutConfig_Weapon& AttachmentsToApply, int32 WeaponIndex)
{
	FInfantryWeaponState& WeaponToApplyTo = Loadout.WeaponSystem.InfantryWeaponSystem.WeaponState_FP[WeaponIndex];
	for (auto& Slot : AttachmentsToApply.WeaponAttachments)
	{
		const EAttachmentSlot& SlotType = Slot.Key;
		const FPlayerLoadoutConfig_WeaponAttachment& AttachmentConfig = Slot.Value;
		FWeaponAttachmentState& RuntimeSlotState = WeaponToApplyTo.WeaponAttachmentStates.FindOrAdd(SlotType);

		Init_Attachment(RuntimeSlotState, WeaponToApplyTo, SlotType);
		UpdateAttachment(RuntimeSlotState, AttachmentConfig.AttachmentID, GetBaseWeaponState(WeaponIndex).WeaponID, SlotType);
	}
}

void UWeaponLogicComponent::UpdateWeaponMesh(FName WeaponID, TWeakObjectPtr<USkeletalMeshComponent>& WeaponMeshComp)
{
	const FInfantryWeaponData& WeaponData = *UBS2FunctionLibrary::GetDataSubsystem(this)->GetInfantryWeaponDataRow(WeaponID);
	TWeakObjectPtr<USkeletalMesh> WeaponMesh = WeaponData.WeaponClassificationData.WeaponMesh.LoadSynchronous();
	WeaponMeshComp->SetSkeletalMesh(WeaponMesh.Get());
}

void UWeaponLogicComponent::UpdateGadgetMesh(FName GadgetID, TWeakObjectPtr<UStaticMeshComponent>& GadgetMeshComp)
{
	const FGadgetData& GadgetData = *UBS2FunctionLibrary::GetDataSubsystem(this)->GetGadgetDataRow(GadgetID);
	TWeakObjectPtr<UStaticMesh> GadgetMesh = GadgetData.GadgetMesh.LoadSynchronous();
	GadgetMeshComp->SetStaticMesh(GadgetMesh.Get());
}

void UWeaponLogicComponent::UpdateGadgetVisibility(int32 GadgetIndex, bool Hide)
{
	Loadout.Gadgets[GadgetIndex].HeldMesh_FP->SetHiddenInGame(Hide);
}

void UWeaponLogicComponent::UpdateWeaponData(int32 WeaponIndex, FName WeaponID, FInfantryWeaponState WeaponState)
{
	Loadout.WeaponSystem.BaseWeaponSystem.Weapons[WeaponIndex].WeaponID = WeaponID;
	StaticWeaponDataCache[WeaponIndex] = UBS2FunctionLibrary::GetDataSubsystem(this)->GetInfantryWeaponDataRow(WeaponID);

	//add a bool or an int if it should fp, tp or both
	Loadout.WeaponSystem.InfantryWeaponSystem.WeaponState_FP[WeaponIndex] = WeaponState;
}

void UWeaponLogicComponent::UpdateScopeCamera()
{
	//called on equip weapon
	Loadout.WeaponSystem.ScopeCamera->AttachToComponent(Loadout.WeaponSystem.InfantryWeaponSystem.WeaponState_FP[GetCII()].WeaponMesh.Get(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, true), FName("PIP"));

	if (Loadout.WeaponSystem.InfantryWeaponSystem.WeaponState_FP[GetCII()].WeaponAttachmentStates.Find(EAttachmentSlot::Scope))
	{
		FWeaponAttachmentState& WeaponAttachmentState = *Loadout.WeaponSystem.InfantryWeaponSystem.WeaponState_FP[GetCII()].WeaponAttachmentStates.Find(EAttachmentSlot::Scope);
		int32 PIPMatIndex = UBS2FunctionLibrary::GetDataSubsystem(this)->GetWeaponAttachmentDataRow(WeaponAttachmentState.BaseAttachmentState.AttachmentID)->WeaponSightData.PIPMaterialIndex;
		if (PIPMatIndex < 0) { return; }
		UMaterialInstanceDynamic* MID = WeaponAttachmentState.SpawnedAttachment->CreateDynamicMaterialInstance(PIPMatIndex, WeaponAttachmentState.SpawnedAttachment->GetMaterial(PIPMatIndex));
		WeaponAttachmentState.SpawnedAttachment->SetMaterial(PIPMatIndex, MID);
		MID->SetTextureParameterValue(FName("RenderTarget"), Loadout.WeaponSystem.ScopeCamera->TextureTarget);
	}
}

void UWeaponLogicComponent::UpdateAttachment(FWeaponAttachmentState& RuntimeSlotState, FName AttachmentID, FName WeaponID, EAttachmentSlot AttachmentSlot)
{
	//update attachment mesh
	const FWeaponAttachmentData& AttachmentData = *UBS2FunctionLibrary::GetDataSubsystem(this)->GetWeaponAttachmentDataRow(AttachmentID);
	TWeakObjectPtr<UStaticMesh> AttachmentMesh = AttachmentData.AttachmentClassification.AttachmentMesh.LoadSynchronous();
	const FVector& AttachmentOffset = GetAttachmentDefaultOffset(WeaponID, AttachmentSlot, AttachmentID);

	RuntimeSlotState.SpawnedAttachment->SetStaticMesh(AttachmentMesh.Get());
	RuntimeSlotState.SpawnedAttachment->SetRelativeLocation(AttachmentOffset);
	RuntimeSlotState.BaseAttachmentState.AttachmentID = AttachmentID;
}

void UWeaponLogicComponent::UpdateWeaponCollision(ECollisionChannel CollisionChannel, ECollisionResponse CollisionResponse, int32 WeaponIndex)
{
	Loadout.WeaponSystem.InfantryWeaponSystem.WeaponState_FP[WeaponIndex].WeaponMesh->SetCollisionResponseToChannel(CollisionChannel, CollisionResponse);
	for (auto& Attachment : Loadout.WeaponSystem.InfantryWeaponSystem.WeaponState_FP[WeaponIndex].WeaponAttachmentStates)
	{
		Attachment.Value.SpawnedAttachment.Get()->SetCollisionResponseToChannel(CollisionChannel, CollisionResponse);
	}
	//WeaponSystem.InfantryWeaponSystem.WeaponState_TP[WeaponSystem.BaseWeaponSystem.EquippedWeaponState.CurrentWeaponIndex].WeaponMesh->SetCollisionResponseToChannel(CollisionChannel, CollisionResponse);
}

void UWeaponLogicComponent::UpdateWeaponVisibility(int32 WeaponIndex, bool Hide)
{
	FInfantryWeaponState& Weapon = Loadout.WeaponSystem.InfantryWeaponSystem.WeaponState_FP[WeaponIndex];
	Weapon.WeaponMesh->SetHiddenInGame(Hide);
	for (auto& AttachmentSlot : Weapon.WeaponAttachmentStates)
	{
		AttachmentSlot.Value.SpawnedAttachment->SetHiddenInGame(Hide);
	}
}

#pragma region Aiming

void UWeaponLogicComponent::StartAim()
{
	const FInfantryWeaponAimData& AimData = GetCurrentWeaponStaticData()->InfantryWeaponAimData;
	if (!AimData.canAim) { return; }
	if (AimData.HideArms)
	{
		TWeakObjectPtr<ACharacter_Base> Character = Cast<ACharacter_Base>(GetOwner());
		USkeletalMeshComponent* FPArms = Character->FPArms;
		FPArms->SetVisibility(false);
		FString SocketString = FString::Printf(TEXT("Socket_%s_1"), *GetCurrentWeaponRuntime()->WeaponID.ToString());
		FName AttachSocketName = FName(*SocketString);
		if (FPArms->DoesSocketExist(AttachSocketName))
		{
			GetCurrentInfantryWeaponState_FP().WeaponMesh->AttachToComponent(Character->FPArms, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, true), AttachSocketName);
		}
	}
	Loadout.WeaponSystem.isAiming = true;
}

void UWeaponLogicComponent::StopAim()
{
	const FInfantryWeaponAimData& AimData = GetCurrentWeaponStaticData()->InfantryWeaponAimData;
	if (!AimData.canAim) { return; }
	if (!Loadout.WeaponSystem.isAiming) { return; }
	if (AimData.HideArms)
	{
		TWeakObjectPtr<ACharacter_Base> Character = Cast<ACharacter_Base>(GetOwner());
		USkeletalMeshComponent* FPArms = Character->FPArms;
		FPArms->SetVisibility(true);
		FString SocketString = FString::Printf(TEXT("Socket_%s"), *GetCurrentWeaponRuntime()->WeaponID.ToString());
		FName AttachSocketName = FName(*SocketString);
		GetCurrentInfantryWeaponState_FP().WeaponMesh->AttachToComponent(Character->FPArms, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, true), AttachSocketName);
	}
	Loadout.WeaponSystem.isAiming = false;
}

# pragma endregion 

void UWeaponLogicComponent::Rangefinder()
{
	FHitResult OutHit;
	UBS2FunctionLibrary::PerformWeaponLineTrace(this, Cast<ACharacter_Base>(GetOwner())->FPCamera->GetComponentTransform(), OutHit, { GetOwner() }, false);
	TWeakObjectPtr<USkeletalMeshComponent>& WeaponMesh = Loadout.WeaponSystem.InfantryWeaponSystem.WeaponState_FP[GetCII()].WeaponMesh;

	Loadout.WeaponSystem.BaseWeaponSystem.EquippedWeaponState.RaycastData.RangefinderData = OutHit;
	Loadout.WeaponSystem.BaseWeaponSystem.EquippedWeaponState.RaycastData.MuzzleAimDirections[0] = UBS2FunctionLibrary::GetAimDirectionFromMuzzle(OutHit, FName("Muzzle"), WeaponMesh);
}

#pragma region WeaponFire

void UWeaponLogicComponent::HandleStartFire()
{
	FWeapon_Runtime& CurrentWeapon = *GetCurrentWeaponRuntime();

	if (!CurrentWeapon.WeaponState.canFire)
	{
		if (CurrentWeapon.WeaponState.CurrentAmmoinMag <= 0)
		{
			DryFire();
		}
		return;
	}

	switch (CurrentWeapon.WeaponState.CurrentFireMode)
	{
		case EFireMode::Single:
			if (!CurrentWeapon.WeaponState.isFiring)
			{
				HandleShootProjectileActor();
			}
			break;
		case EFireMode::Burst:
			break;
		case EFireMode::Auto:
			break;
	}

}

void UWeaponLogicComponent::StartFire()
{
}

void UWeaponLogicComponent::HandleShootProjectileActor()
{
	const FInfantryWeaponData& StaticWeaponData = *GetCurrentWeaponStaticData();
	TWeakObjectPtr<AProjectile_Base> FiredProjectile = nullptr;
	if (GetCurrentWeaponStaticData()->InfantryWeaponAmmoData.isProjectileMounted)
	{

	}
	else
	{
		FiredProjectile = UBS2FunctionLibrary::GetProjectileSystem(this)->AcquireProjectileFromPool(StaticWeaponData.WeaponFirePerformanceData.MunitionID);
		FiredProjectile->MoveIgnoreActorAdd(GetOwner());
		FVector& AimDirection = Loadout.WeaponSystem.BaseWeaponSystem.EquippedWeaponState.RaycastData.MuzzleAimDirections[0];
		FTransform MuzzleTransform = UBS2FunctionLibrary::GetMuzzleTransform(FName("Muzzle"), GetCurrentInfantryWeaponState_FP().WeaponMesh);
		FiredProjectile->SetActorTransform(MuzzleTransform);
		FiredProjectile->FireProjectile(AimDirection);
	}
}

void UWeaponLogicComponent::CeaseFire()
{
	FWeapon_Runtime& CurrentWeapon = *GetCurrentWeaponRuntime();

	CurrentWeapon.WeaponState.isFiring = false;
}

void UWeaponLogicComponent::DryFire()
{
	//play whatever dryfire sound and animation
}

void UWeaponLogicComponent::FireWeapon()
{
	const FInfantryWeaponData* StaticWeaponData = GetCurrentWeaponStaticData();
	FWeapon_Runtime& CurrentWeapon = *GetCurrentWeaponRuntime();	

	switch(StaticWeaponData->WeaponFirePerformanceData.WeaponFireType)
	{
		case EWeaponFireType::SimProjectile:
			break;
		case EWeaponFireType::ActorProjectile:
			break;
		case EWeaponFireType::VFX:
			break;
		case EWeaponFireType::Hitscan:
			break;
	}
}

#pragma endregion

#pragma region WeaponReload

void UWeaponLogicComponent::ReloadWeapon()
{
	FWeapon_Runtime& CurrentWeapon = *GetCurrentWeaponRuntime();
	const FInfantryWeaponData* StaticWeaponData = GetCurrentWeaponStaticData();
	FInfantryWeaponState& InfantryWeaponState_FP = Loadout.WeaponSystem.InfantryWeaponSystem.WeaponState_FP[GetCII()];
	ReloadEndedDelegate.BindUObject(this, &UWeaponLogicComponent::OnReloadFinished);

	if (CurrentWeapon.WeaponState.CurrentReserveAmmo > 0)
	{ 
		CurrentWeapon.WeaponState.isReloading = true;
		bool bEmptyMag = CurrentWeapon.WeaponState.CurrentAmmoinMag <= 0;

		if (!StaticWeaponData->InfantryWeaponAmmoData.ProjectileBoneToHide.IsNone())
		{
			InfantryWeaponState_FP.WeaponMesh->UnHideBoneByName(StaticWeaponData->InfantryWeaponAmmoData.ProjectileBoneToHide);
		}

		TWeakObjectPtr<ACharacter_Base> Character = Cast<ACharacter_Base>(GetOwner());
		USkeletalMeshComponent* FPArms = Character->FPArms;
		
		/**
		FString SocketString = FString::Printf(TEXT("Socket_%s_R"), *GetCurrentWeaponRuntime()->WeaponID.ToString());
		FName AttachSocketName = FName(*SocketString);
		if (FPArms->DoesSocketExist(AttachSocketName))
		{
			InfantryWeaponState_FP.WeaponMesh->AttachToComponent(Character->FPArms, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, true), AttachSocketName);
		}
		**/

		TSoftObjectPtr<UAnimMontage> FPReloadMontage = bEmptyMag ? StaticWeaponData->InfantryWeaponAnimData.FPWeaponAnimData.ReloadEmptyWeaponMontage : StaticWeaponData->InfantryWeaponAnimData.FPWeaponAnimData.ReloadWeaponMontage;
		TSoftObjectPtr<UAnimSequence> WeaponReloadAnim = bEmptyMag ? StaticWeaponData->InfantryWeaponAnimData.WeaponAnimData.WeaponReloadEmpty : StaticWeaponData->InfantryWeaponAnimData.WeaponAnimData.WeaponReload;
		FPReloadMontage.LoadSynchronous();
		WeaponReloadAnim.LoadSynchronous();

		InfantryWeaponState_FP.WeaponMesh->PlayAnimation(WeaponReloadAnim.Get(), false);

		TWeakObjectPtr<UAnimInstance> FPArmsAnimInstance = Cast<ACharacter_Base>(GetOwner())->FPArms->GetAnimInstance();
		FPArmsAnimInstance->Montage_Play(FPReloadMontage.Get(), 1.0f);
		FPArmsAnimInstance->Montage_SetEndDelegate(ReloadEndedDelegate, FPReloadMontage.Get());
	}
}

void UWeaponLogicComponent::OnReloadFinished(UAnimMontage* Montage, bool bInterrupted)
{
	UE_LOG(LogTemp, Warning, TEXT("[WLC::OnReloadFinished"));
	FWeaponStats_Runtime& CurrentWeaponStats = GetCurrentWeaponStats();
	FWeapon_Runtime& CurrentWeapon = *GetCurrentWeaponRuntime();
	const FInfantryWeaponData* StaticWeaponData = GetCurrentWeaponStaticData();
	FInfantryWeaponState& InfantryWeaponState_FP = Loadout.WeaponSystem.InfantryWeaponSystem.WeaponState_FP[GetCII()];

	TWeakObjectPtr<ACharacter_Base> Character = Cast<ACharacter_Base>(GetOwner());
	USkeletalMeshComponent* FPArms = Character->FPArms;
	
	/**
	FString ReloadSocketString = FString::Printf(TEXT("Socket_%s_R"), *GetCurrentWeaponRuntime()->WeaponID.ToString());
	FString SocketString = FString::Printf(TEXT("Socket_%s"), *GetCurrentWeaponRuntime()->WeaponID.ToString());
	FName ReloadAttachSocketName = FName(*ReloadSocketString);
	FName AttachSocketName = FName(*SocketString);
	if (FPArms->DoesSocketExist(ReloadAttachSocketName))			//assumes attached to ReloadSocket
	{
		InfantryWeaponState_FP.WeaponMesh->AttachToComponent(Character->FPArms, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, true), AttachSocketName);
	}
	if (!StaticWeaponData->InfantryWeaponAmmoData.ProjectileBoneToHide.IsNone())
	{
		InfantryWeaponState_FP.WeaponMesh->HideBoneByName(StaticWeaponData->InfantryWeaponAmmoData.ProjectileBoneToHide, EPhysBodyOp::PBO_None);
	}
	**/

	int32 NewCAM, NewCRA;
	UBS2FunctionLibrary::CalculateReload(CurrentWeaponStats.MagSize, CurrentWeapon.WeaponState.CurrentAmmoinMag, CurrentWeapon.WeaponState.CurrentReserveAmmo, NewCAM, NewCRA);
	CurrentWeapon.WeaponState.CurrentAmmoinMag = NewCAM;
	CurrentWeapon.WeaponState.CurrentReserveAmmo = NewCRA;
	CurrentWeapon.WeaponState.isReloading = false;
}

#pragma endregion

void UWeaponLogicComponent::AutoSwitchItem()
{
	//use to auto switch/revolve/rotate between items in a list
	switch (Loadout.CurrentSlot)
	{
		case ELoadoutSlot::PrimaryWeapon:
			HandleSwitchItem(ELoadoutSlot::SecondaryWeapon);
			break;
		case ELoadoutSlot::SecondaryWeapon:
		case ELoadoutSlot::Gadget1:
		case ELoadoutSlot::Gadget2:
		case ELoadoutSlot::Gadget3:
			HandleSwitchItem(ELoadoutSlot::PrimaryWeapon);
			break;
	}
}

void UWeaponLogicComponent::HandleSwitchItem(ELoadoutSlot NewLoadoutSlot)
{
	//main switch weapon function, can be used to directly equip a given index (manual) or auto increment (via SwitchWeapon_AutoIncrement)
	if (NewLoadoutSlot == Loadout.CurrentSlot)		{ return;	}
	ELoadoutSlot OldLoadoutSlot = Loadout.CurrentSlot;

	int32 OldRawPosition = GetArrayIndex(OldLoadoutSlot);
	int32 NewItemIndex = GetArrayIndex(NewLoadoutSlot);
	ECharacterItemType OldItemType = GetCategoryForSlot(OldLoadoutSlot);
	ECharacterItemType NewItemType = GetCategoryForSlot(NewLoadoutSlot);
	
	if (OldItemType == ECharacterItemType::Gadget)
	{
		OldItemType = GetActualGadgetItemType(OldRawPosition);
	}
	if (NewItemType == ECharacterItemType::Gadget)
	{
		NewItemType = GetActualGadgetItemType(NewItemIndex);
	}
	
	// NOW that OldItemType is resolved, get the TRUE resolved index 
	Loadout.PreviousItemIndex = (OldItemType == ECharacterItemType::Weapon) ? GetWeaponIndexForSlot(OldLoadoutSlot) : GetGadgetIndexForSlot(OldLoadoutSlot);
	
	Loadout.CurrentSlot = NewLoadoutSlot;
	
	switch (OldItemType)
	{
		case ECharacterItemType::Weapon:
			switch (NewItemType)
			{
				case ECharacterItemType::Weapon:
					UnequipBlendOutDelegate.BindUObject(this, &UWeaponLogicComponent::OnUnequipWeapon_BlendOutToWeapon);
					UnequipWeapon(Loadout.PreviousItemIndex);
					break;
				case ECharacterItemType::Gadget:
					UnequipBlendOutDelegate.BindUObject(this, &UWeaponLogicComponent::OnUnequipWeapon_BlendOutToGadget);
					UnequipWeapon(Loadout.PreviousItemIndex);
					break;
			}
			break;
		case ECharacterItemType::Gadget:
			switch (NewItemType)
			{
				case ECharacterItemType::Gadget:
					UnequipGadget(Loadout.PreviousItemIndex);
					break;
				case ECharacterItemType::Weapon:
					UnequipGadget(Loadout.PreviousItemIndex);
					break;
			}
			break;
	}
}

void UWeaponLogicComponent::TransitionFromItem(int32 PreviousItemIndex, ECharacterItemType LoadoutCategory)
{
	//basically unequip for everything non-anim related
	TWeakObjectPtr<ACharacter_Base> Character = Cast<ACharacter_Base>(GetOwner());
	Character->FPArms->SetVisibility(false);
	switch (LoadoutCategory)
	{
		case ECharacterItemType::Weapon:
			UpdateWeaponVisibility(PreviousItemIndex, true);
			break;
		case ECharacterItemType::Gadget:
			UpdateGadgetVisibility(PreviousItemIndex, true);
			break;
	}
	GetWorld()->GetTimerManager().SetTimer(SwitchWeaponTimer, [this, Character]()
	{
		Character->FPArms->SetVisibility(true);
	}, 0.05f, false);
	
	TransitionToItem();
}

void UWeaponLogicComponent::TransitionToItem()
{
	//basically equip for everything non-anim related
	int32 ItemIndex;
	switch (Loadout.CurrentSlot)
	{
		case ELoadoutSlot::PrimaryWeapon:
		case ELoadoutSlot::SecondaryWeapon:
			ItemIndex = GetWeaponIndexForSlot(Loadout.CurrentSlot);
			EquipWeapon(ItemIndex, false);
			break;
		case ELoadoutSlot::Gadget1:
		case ELoadoutSlot::Gadget2:
		case ELoadoutSlot::Gadget3:
			ItemIndex = GetWeaponIndexForSlot(Loadout.CurrentSlot);
			if (ItemIndex == INDEX_NONE)
			{
				//equip gadget
				ItemIndex = GetGadgetIndexForSlot(Loadout.CurrentSlot);
				EquipGadget(ItemIndex);
			}
			else
			{
				//equip weapon gadget works the same as any other weapon
				EquipWeapon(ItemIndex, false);
			}
			break;
	}
}

void UWeaponLogicComponent::UnequipWeapon(int32 PreviousWeaponIndex)
{
	TSoftObjectPtr<UAnimMontage> FPUnequipItemMontage = StaticWeaponDataCache[PreviousWeaponIndex]->InfantryWeaponAnimData.FPWeaponAnimData.UnequipWeaponMontage;
	FPUnequipItemMontage.LoadSynchronous();
	ECharacterItemType CurrentItemType = GetCategoryForSlot(Loadout.CurrentSlot);
	if (!FPUnequipItemMontage.Get())
	{
		TransitionFromItem(PreviousWeaponIndex, ECharacterItemType::Weapon);
		return;
	}
	TWeakObjectPtr<UAnimInstance> FPArmsAnimInstance = Cast<ACharacter_Base>(GetOwner())->FPArms->GetAnimInstance();
	
	FPArmsAnimInstance->Montage_Play(FPUnequipItemMontage.Get(), 1.0f, EMontagePlayReturnType::MontageLength, 0.0f);
	FPArmsAnimInstance->Montage_SetBlendingOutDelegate(UnequipBlendOutDelegate, FPUnequipItemMontage.Get());		//BINDING SHOULDVE HAPPENED BEFORE THIS FUNCTION IS CALLED
}

void UWeaponLogicComponent::OnUnequipWeapon_BlendOutToWeapon(UAnimMontage* Montage, bool bInterrupted)
{
	//old slot was weapon, new slot is weapon
	TWeakObjectPtr<ACharacter_Base> Character = Cast<ACharacter_Base>(GetOwner());
	TWeakObjectPtr<UAnimInstance> FPArmsAnimInstance = Character->FPArms->GetAnimInstance();
	TSoftObjectPtr<UAnimMontage> FPUnequipWeaponMontage = StaticWeaponDataCache[Loadout.PreviousItemIndex]->InfantryWeaponAnimData.FPWeaponAnimData.UnequipWeaponMontage;
	if (!FPUnequipWeaponMontage.Get())
	{
		EquipWeapon(GetWeaponIndexForSlot(Loadout.CurrentSlot), false);
		return;
	}
	FPArmsAnimInstance->Montage_Play(FPUnequipWeaponMontage.Get(), 0.0f, EMontagePlayReturnType::MontageLength, FPUnequipWeaponMontage->GetPlayLength());

	TransitionFromItem(Loadout.PreviousItemIndex, ECharacterItemType::Weapon);	//remember, this function calls equip weapon in it
}

void UWeaponLogicComponent::OnUnequipWeapon_BlendOutToGadget(UAnimMontage* Montage, bool bInterrupted)
{
	//old slot was weapon, new slot is gadget
	TWeakObjectPtr<ACharacter_Base> Character = Cast<ACharacter_Base>(GetOwner());
	TWeakObjectPtr<UAnimInstance> FPArmsAnimInstance = Character->FPArms->GetAnimInstance();
	TSoftObjectPtr<UAnimMontage> FPUnequipWeaponMontage = StaticWeaponDataCache[Loadout.PreviousItemIndex]->InfantryWeaponAnimData.FPWeaponAnimData.UnequipWeaponMontage;
	if (!FPUnequipWeaponMontage.Get())
	{
		EquipGadget(GetGadgetIndexForSlot(Loadout.CurrentSlot));	
		return;
	}
	FPArmsAnimInstance->Montage_Play(FPUnequipWeaponMontage.Get(), 0.0f, EMontagePlayReturnType::MontageLength, FPUnequipWeaponMontage->GetPlayLength());
	TransitionFromItem(Loadout.PreviousItemIndex, ECharacterItemType::Weapon);
}

void UWeaponLogicComponent::EquipWeapon(int32 WeaponIndex, bool InitialEquip)
{
	TWeakObjectPtr<ACharacter_Base> Character = Cast<ACharacter_Base>(GetOwner());
	TWeakObjectPtr<UAnimInstance> FPArmsAnimInstance = Character->FPArms->GetAnimInstance();
	const FInfantryWeaponAnimData& AnimData = StaticWeaponDataCache[WeaponIndex]->InfantryWeaponAnimData;
	FInfantryWeaponState& InfantryWeaponState_FP = Loadout.WeaponSystem.InfantryWeaponSystem.WeaponState_FP[WeaponIndex];
	TObjectPtr<USkeletalMeshComponent> NewWeaponMesh = Loadout.WeaponSystem.InfantryWeaponSystem.WeaponState_FP[WeaponIndex].WeaponMesh.Get();
	UpdateWeaponVisibility(WeaponIndex, false);
	TSoftObjectPtr<UAnimMontage> FPEquipWeaponMontage;
	if (InitialEquip)
	{
		//only do weapon mesh animation on initial equip
		TSoftObjectPtr<UAnimSequence> WeaponEquipAnim = AnimData.WeaponAnimData.WeaponEquip;
		WeaponEquipAnim.LoadSynchronous();
		NewWeaponMesh->PlayAnimation(WeaponEquipAnim.Get(), false);
		
		FPEquipWeaponMontage = AnimData.FPWeaponAnimData.InitialEquipWeaponMontage;
	}
	else
	{
		FPEquipWeaponMontage = AnimData.FPWeaponAnimData.EquipWeaponMontage;
	}

	FPEquipWeaponMontage.LoadSynchronous();
	FPArmsAnimInstance->Montage_Play(FPEquipWeaponMontage.Get(), 1.0f);
	IAnims::Execute_OnEquipWeapon_FP(FPArmsAnimInstance.Get(), AnimData.FPWeaponAnimData);
	if (!StaticWeaponDataCache[WeaponIndex]->InfantryWeaponAmmoData.ProjectileBoneToHide.IsNone())
	{
		InfantryWeaponState_FP.WeaponMesh->HideBoneByName(StaticWeaponDataCache[WeaponIndex]->InfantryWeaponAmmoData.ProjectileBoneToHide, EPhysBodyOp::PBO_None);
	}
	UpdateScopeCamera();
}

void UWeaponLogicComponent::ToggleFireMode()
{
	FWeaponStats_Runtime& CurrentWeaponStats = GetCurrentWeaponStats();
	FWeaponFireModeData& CurrentFireModeData = CurrentWeaponStats.FireModeData;
	FWeapon_Runtime& CurrentWeapon = *GetCurrentWeaponRuntime();
	EFireMode& CurrentFireMode = CurrentWeapon.WeaponState.CurrentFireMode;

	switch (CurrentFireMode)
	{
		case EFireMode::Single:
			if (CurrentFireModeData.canFullAuto)
			{
				CurrentFireMode = EFireMode::Auto;
			}
			else if (CurrentFireModeData.canBurstFire)
			{
				CurrentFireMode = EFireMode::Burst;
			}
			break;
		case EFireMode::Burst:
			if (CurrentFireModeData.canSingleFire)
			{
				CurrentFireMode = EFireMode::Single;
			}
			else if (CurrentFireModeData.canFullAuto)
			{
				CurrentFireMode = EFireMode::Auto;
			}
			break;
		case EFireMode::Auto:
			if (CurrentFireModeData.canBurstFire)
			{
				CurrentFireMode = EFireMode::Burst;
			}
			else if (CurrentFireModeData.canSingleFire)
			{
				CurrentFireMode = EFireMode::Single;
			}
			break;
	}
}

#pragma region WeaponScope

void UWeaponLogicComponent::ToggleScope()
{
	if (!Loadout.WeaponSystem.isAiming) { return; }
	const FWeaponAttachmentData& WeaponAttachmentData = *UBS2FunctionLibrary::GetDataSubsystem(this)->GetWeaponAttachmentDataRow(GetCurrentAttachmentInSlot(EAttachmentSlot::Scope).BaseAttachmentState.AttachmentID);
	int32& CurrentOpticIndex = Loadout.WeaponSystem.InfantryWeaponSystem.WeaponState_FP[GetCII()].CurrentOpticIndex;
	if (WeaponAttachmentData.WeaponSightData.OpticIDs.Num() > 1)
	{
		int32 TotalOpticLevels = WeaponAttachmentData.WeaponSightData.OpticIDs.Num();
		UBS2FunctionLibrary::UpdateOpticIndex(TotalOpticLevels, CurrentOpticIndex);
		UpdateScope(CurrentOpticIndex);
	}
}

void UWeaponLogicComponent::UpdateScope(int32 NewOpticIndex)
{
	float& DefaultFOV = UBS2FunctionLibrary::GetDataSubsystem(this)->GetWeaponDefaults()->WeaponDefaults.ScopeCameraFOV;
	FName& ScopeID = GetCurrentAttachmentInSlot(EAttachmentSlot::Scope).BaseAttachmentState.AttachmentID;
	if (ScopeID == NAME_None) { return; }
	FName OpticID = UBS2FunctionLibrary::GetDataSubsystem(this)->GetWeaponAttachmentDataRow(ScopeID)->WeaponSightData.OpticIDs[NewOpticIndex];
	const FOpticData& OpticData = *UBS2FunctionLibrary::GetDataSubsystem(this)->GetOpticDataRow(OpticID);

	//post process settings for scope camera, if any. If no post process settings are defined, set capture source to HDR scene color to avoid weirdness with the render target.
	if (OpticData.OpticPPSettings.WeightedBlendables.Array.Num() > 0)
	{
		if (Loadout.WeaponSystem.ScopeCamera->CaptureSource != ESceneCaptureSource::SCS_FinalColorLDR)
		{
			Loadout.WeaponSystem.ScopeCamera->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
		}
	}
	else if (Loadout.WeaponSystem.ScopeCamera->CaptureSource != ESceneCaptureSource::SCS_SceneColorHDR)
	{
		Loadout.WeaponSystem.ScopeCamera->CaptureSource = ESceneCaptureSource::SCS_SceneColorHDR;
	}

	UBS2FunctionLibrary::HandleUpdateOptic(DefaultFOV, OpticData.ZoomMagnification, Loadout.WeaponSystem.ScopeCamera->FOVAngle, OpticData.OpticPPSettings, Loadout.WeaponSystem.ScopeCamera->PostProcessSettings, Loadout.WeaponSystem.ScopeCamera->PostProcessBlendWeight);
}

#pragma endregion

#pragma region WeaponAttachments

void UWeaponLogicComponent::UpdateCurrentWeaponStats(int32 WeaponIndex)
{
	FWeaponStats_Runtime& RuntimeStats = Loadout.WeaponSystem.InfantryWeaponSystem.CurrentWeaponStats[WeaponIndex];
	FName& WeaponID = Loadout.WeaponSystem.BaseWeaponSystem.Weapons[WeaponIndex].WeaponID;
	const FInfantryWeaponData* StaticWeaponData = StaticWeaponDataCache[WeaponIndex];

	RuntimeStats.AimInSpeed = StaticWeaponData->InfantryWeaponAimData.DefaultAimInSpeed;
	RuntimeStats.AimOutSpeed = StaticWeaponData->InfantryWeaponAimData.DefaultAimOutSpeed;
	RuntimeStats.SightDistance = StaticWeaponData->InfantryWeaponAimData.DefaultSightDistance;
	RuntimeStats.MagSize = StaticWeaponData->InfantryWeaponAmmoData.BaseAmmoData.MagSize;
	RuntimeStats.MaxReserveAmmo = StaticWeaponData->InfantryWeaponAmmoData.BaseAmmoData.MaxReserveAmmo;

	TMap<EAttachmentSlot, FWeaponAttachmentState>& WeaponAttachmentStates = Loadout.WeaponSystem.InfantryWeaponSystem.WeaponState_FP[WeaponIndex].WeaponAttachmentStates;
	for (auto& SlotPair : WeaponAttachmentStates)
	{
		FName& AttachmentID = SlotPair.Value.BaseAttachmentState.AttachmentID;
		if (AttachmentID.IsNone()) continue;
		const FWeaponAttachmentData* AttachmentData = UBS2FunctionLibrary::GetDataSubsystem(this)->GetWeaponAttachmentDataRow(AttachmentID);

		for (auto& ModifierPair : AttachmentData->AttachmentModifiers)
		{
			ApplyAttachmentModifier(RuntimeStats, ModifierPair.Key, ModifierPair.Value);
		}
	}
}

void UWeaponLogicComponent::ApplyAttachmentModifier(FWeaponStats_Runtime& RuntimeStats, EWeaponStat WeaponStat, const FStatModifierData& Modifier)
{
	using FloatPtr = float FWeaponStats_Runtime::*;				//FloatPtr = float*
	using IntPtr = int32 FWeaponStats_Runtime::*;				//IntPtr = int32*
	using BoolPtr = bool FWeaponStats_Runtime::*;

	// Local struct that holds either a float or int pointer-to-member
	// One of these will always be nullptr depending on the stat's type
	struct FStatTarget
	{
		FloatPtr FloatMember = nullptr;
		IntPtr IntMember = nullptr;
		BoolPtr BoolMember = nullptr;
	};

	// This map is the only place you touch when adding a new stat
	// "static const" means it's built ONCE for the lifetime of the program
	// not rebuilt every time this function is called
	// Key   = which stat we want to affect (the enum)
	// Value = which field on FWeaponStats_Runtime corresponds to that stat
	static const TMap<EWeaponStat, FStatTarget> StatTargets =
	{
		{ EWeaponStat::ADSInSpeed,      
			{ 
				&FWeaponStats_Runtime::AimInSpeed,     
				nullptr,
			} 
		},
		{ EWeaponStat::ADSOutSpeed,
			{
				&FWeaponStats_Runtime::AimOutSpeed,
				nullptr,
				nullptr
			}
		},
		{ EWeaponStat::MuzzleVelocity, 
			{ 
				&FWeaponStats_Runtime::MuzzleVelocity, 
				nullptr,
				nullptr
			} 
		},
		{ EWeaponStat::BaseDamage,	   
			{ 
				&FWeaponStats_Runtime::BaseDamage,    
				nullptr,
				nullptr
			}
		},
		{ EWeaponStat::MagSize,        
			{ 
				nullptr, 
				&FWeaponStats_Runtime::MagSize,
				nullptr
			} 
		},
		{ EWeaponStat::MaxReserveAmmo, 
			{ 
				nullptr, 
				&FWeaponStats_Runtime::MaxReserveAmmo,
				nullptr
			}
		},
	};

	const FStatTarget* StatTarget = StatTargets.Find(WeaponStat);

	if (StatTarget->FloatMember)
	{
		float& FloatValue = RuntimeStats.*StatTarget->FloatMember;
		FloatValue = Modifier.ApplyToValue(FloatValue);
	}
	else if (StatTarget->IntMember)
	{
		int32& IntValue = RuntimeStats.*StatTarget->IntMember;
		IntValue = FMath::RoundToInt(Modifier.ApplyToValue((float)IntValue));
	}
	else if (StatTarget->BoolMember)
	{
		bool& BoolValue = RuntimeStats.*StatTarget->BoolMember;
		if (Modifier.Operation == EModifierOp::Set)
		{
			BoolValue = !FMath::IsNearlyZero(Modifier.ModifierValue);
		}
	}
}

float UWeaponLogicComponent::CalculateFinalStatValue(float BaseValue, TArray<FStatModifierData>& ModifierArray)
{
	float CurrentValue = BaseValue;
	for (FStatModifierData& Modifier : ModifierArray)
	{
		CurrentValue = Modifier.ApplyToValue(CurrentValue);
	}
	return CurrentValue;
}

#pragma endregion 

void UWeaponLogicComponent::UnequipGadget(int32 PreviousGadgetIndex)
{
	TSoftObjectPtr<UAnimMontage> FPUnequipItemMontage = StaticGadgetDataCache[PreviousGadgetIndex]->GadgetAnimData.UnequipGadget;
	FPUnequipItemMontage.LoadSynchronous();
	ECharacterItemType CurrentItemType = GetCategoryForSlot(Loadout.CurrentSlot);
	if (!FPUnequipItemMontage.Get())
	{
		TransitionFromItem(PreviousGadgetIndex, ECharacterItemType::Gadget);
		return;
	}
	
	TWeakObjectPtr<UAnimInstance> FPArmsAnimInstance = Cast<ACharacter_Base>(GetOwner())->FPArms->GetAnimInstance();
	
	FPArmsAnimInstance->Montage_Play(FPUnequipItemMontage.Get(), 1.0f, EMontagePlayReturnType::MontageLength, 0.0f);
	FPArmsAnimInstance->Montage_SetBlendingOutDelegate(UnequipBlendOutDelegate, FPUnequipItemMontage.Get());		//BINDING SHOULDVE HAPPENED BEFORE THIS FUNCTION IS CALLED
}

void UWeaponLogicComponent::EquipGadget(int32 GadgetIndex)
{
	TWeakObjectPtr<ACharacter_Base> Character = Cast<ACharacter_Base>(GetOwner());
	TWeakObjectPtr<UAnimInstance> FPArmsAnimInstance = Character->FPArms->GetAnimInstance();
	const FGadgetData& GadgetData = *StaticGadgetDataCache[GadgetIndex];
	const FGadgetAnimData& AnimData = GadgetData.GadgetAnimData;
	TObjectPtr<UStaticMeshComponent> NewGadgetMesh = Loadout.Gadgets[GadgetIndex].HeldMesh_FP.Get();
	UpdateGadgetVisibility(GadgetIndex, false);
	
	AnimData.EquipGadget.LoadSynchronous();
	EquipGadgetBlendOutDelegate.BindUObject(this, &UWeaponLogicComponent::OnEquipGadget_BlendOut);
	FPArmsAnimInstance->Montage_Play(AnimData.EquipGadget.Get(), 1.0f);
	FPArmsAnimInstance->Montage_SetBlendingOutDelegate(EquipGadgetBlendOutDelegate, AnimData.EquipGadget.Get());
}

void UWeaponLogicComponent::HandleDeployGadgetInput()
{
	int32 GadgetIndex = GetCII();
	FGadgetState& GadgetState = Loadout.Gadgets[GadgetIndex];
	const FGadgetData& GadgetData = *StaticGadgetDataCache[GadgetIndex];
	
	switch (GadgetData.GadgetType)
	{
		case EGadgetType::Gadget:
			StartDeployGadget();
			break;
		case EGadgetType::Vehicle:
			//vehicle is a wierd case where only 1 should be deployed at a time so therefore use it
			if (GadgetState.ActivePlacedInstances.Num() > 0)
			{
				UseGadget();
			}
			else
			{
				StartDeployGadget();
			}
			break;
	}
}

void UWeaponLogicComponent::OnEquipGadget_BlendOut(UAnimMontage* Montage, bool bInterrupted)
{
	int32 GadgetIndex = GetCII();
	const FGadgetData& GadgetData = *StaticGadgetDataCache[GadgetIndex];
	FGadgetState& GadgetState = Loadout.Gadgets[GadgetIndex];
	
	if (GadgetData.GadgetType == EGadgetType::Vehicle && GadgetState.ActivePlacedInstances.Num() > 0)
	{
		if (GadgetData.AutoUse)
		{
			UseGadget();
		}
	}
	
	if (GadgetData.AutoDrop)
	{
		StartDeployGadget();
	}
}

void UWeaponLogicComponent::StartDeployGadget()
{
	if (GetCategoryForSlot(Loadout.CurrentSlot) != ECharacterItemType::Gadget)	{ return;}
	int32 GadgetIndex = GetCII();
	if (GetActualGadgetItemType(GadgetIndex) == ECharacterItemType::Weapon)	{  return; }
	FGadgetState& GadgetState = Loadout.Gadgets[GadgetIndex];
	const FGadgetData& GadgetData = *StaticGadgetDataCache[GadgetIndex];
	if (GadgetData.GadgetType == EGadgetType::Vehicle && !GadgetState.ActivePlacedInstances.IsEmpty())	{return;}		//if this gadget is a vehicle & there's 1 deployed, dont deploy another
	
	TWeakObjectPtr<ACharacter_Base> Character = Cast<ACharacter_Base>(GetOwner());
	TWeakObjectPtr<UAnimInstance> FPArmsAnimInstance = Character->FPArms->GetAnimInstance();

	const FGadgetAnimData& AnimData = GadgetData.GadgetAnimData;
	AnimData.DeployGadget.LoadSynchronous();
	UnequipBlendOutDelegate.BindUObject(this, &UWeaponLogicComponent::OnStartDeployGadget_BlendOut);
	FPArmsAnimInstance->Montage_Play(AnimData.DeployGadget.Get(), 1.0f);
	FPArmsAnimInstance->Montage_SetBlendingOutDelegate(UnequipBlendOutDelegate, AnimData.DeployGadget.Get());
}

void UWeaponLogicComponent::OnStartDeployGadget_BlendOut(UAnimMontage* Montage, bool bInterrupted)
{
	DeployGadget();
}

void UWeaponLogicComponent::DeployGadget()
{
	//this gadget is NOT a weapon 
	//this is to place/deploy instances of the gadget (place c4, throw down crate, etc)
	int32 GadgetIndex = GetCII();
	FGadgetState& GadgetState = Loadout.Gadgets[GadgetIndex];
	const FGadgetData& GadgetData = *StaticGadgetDataCache[GadgetIndex];
	
	TObjectPtr<APawn> NewGadget = nullptr;
	FActorSpawnParameters GadgetSpawnParams;
	GadgetSpawnParams.Owner = GetOwner();
	GadgetSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	switch (GadgetData.GadgetType)
	{
		case EGadgetType::Gadget:
			break;
		case EGadgetType::Vehicle:
			//vehicle is a wierd case where only 1 should be deployed at a time so therefore dont deploy 1 if 1 is already deployed/placed
			if (!GadgetState.ActivePlacedInstances.IsEmpty())	{return;}
			TObjectPtr<AVehicle_Base> VehicleGadget = GetWorld()->SpawnActorDeferred<AVehicle_Base>(AVehicle_Base::StaticClass(), GetOwner()->GetActorTransform(), GetOwner(), GetOwnerCharacter(), ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
			VehicleGadget->VehicleStartingData.VehicleID = GadgetData.ItemID;
			VehicleGadget->FinishSpawning(GetOwner()->GetActorTransform());
			check(VehicleGadget);
			NewGadget = VehicleGadget;
			break;
	}
	GadgetState.ActivePlacedInstances.Add(NewGadget);
	GadgetState.CurrentInventory--;
	
	if (GadgetData.AutoUse)
	{
		UseGadget();
	}
}

void UWeaponLogicComponent::UseGadget()
{
	//gadget is NOT a weapon
	//calls whatever event on every active placed instance (detonate c4 or control drone for example)
	int32 GadgetIndex = GetCII();
	FGadgetState& GadgetState = Loadout.Gadgets[GadgetIndex];
	const FGadgetData& GadgetData = *StaticGadgetDataCache[GadgetIndex];
	if (GadgetState.ActivePlacedInstances.IsEmpty())	{ return; }
	
	switch (GadgetData.GadgetType)
	{
		case EGadgetType::Gadget:
			break;
		case EGadgetType::Vehicle:
			TObjectPtr<AVehicle_Base> VehicleGadget = Cast<AVehicle_Base>(GadgetState.ActivePlacedInstances[0]);
			if (VehicleGadget->IsInitialized())
			{
				VehicleGadget->AttemptEnterVehicle(GetOwnerCharacter());
			}
			else
			{
				VehicleGadget->OnVehicleInitialized.AddDynamic(this, &UWeaponLogicComponent::OnDeployedVehicleGadgetReady);
			}
			break;
	}
	
}

void UWeaponLogicComponent::OnDeployedVehicleGadgetReady()
{
	int32 GadgetIndex = GetCII();
	FGadgetState& GadgetState = Loadout.Gadgets[GadgetIndex];
	TObjectPtr<AVehicle_Base> VehicleGadget = Cast<AVehicle_Base>(GadgetState.ActivePlacedInstances[0]);
	check (VehicleGadget);
	VehicleGadget->OnVehicleInitialized.RemoveDynamic(this, &UWeaponLogicComponent::OnDeployedVehicleGadgetReady);
	VehicleGadget->AttemptEnterVehicle(GetOwnerCharacter());
}

#pragma region Getters

ECharacterItemType UWeaponLogicComponent::GetCategoryForSlot(ELoadoutSlot LoadoutSlot)
{
	switch (LoadoutSlot)
	{
		case ELoadoutSlot::PrimaryWeapon:
		case ELoadoutSlot::SecondaryWeapon:
			return ECharacterItemType::Weapon;
		case ELoadoutSlot::Gadget1:
		case ELoadoutSlot::Gadget2:
		case ELoadoutSlot::Gadget3:
			return ECharacterItemType::Gadget;		//this is for inventory purposes, so even if gadget is "actually" a weapon this should still return gadget
		case ELoadoutSlot::Grenade:
			return ECharacterItemType::Grenade;
		case ELoadoutSlot::Melee:
			return ECharacterItemType::Melee;
	}
	return ECharacterItemType::Weapon;
}

int32 UWeaponLogicComponent::GetArrayIndex(ELoadoutSlot LoadoutSlot)
{
	//raw slot position
	switch (LoadoutSlot)
	{
		case ELoadoutSlot::PrimaryWeapon:   return 0;
		case ELoadoutSlot::SecondaryWeapon: return 1;
		case ELoadoutSlot::Gadget1:         return 0;
		case ELoadoutSlot::Gadget2:         return 1;
		case ELoadoutSlot::Gadget3:         return 2;
		default:                            return 0; // Grenade, Melee — single-item categories
	}
}

int32 UWeaponLogicComponent::GetWeaponIndexForSlot(ELoadoutSlot LoadoutSlot)
{
	//GETS THE WEAPON INDEX FOR GADGETS THAT ARE ACTUALLY WEAPONS (gadget weapons)
	switch (LoadoutSlot)
	{
		case ELoadoutSlot::PrimaryWeapon:   return 0;
		case ELoadoutSlot::SecondaryWeapon: return 1;
		case ELoadoutSlot::Gadget1:         
		case ELoadoutSlot::Gadget2:
		case ELoadoutSlot::Gadget3:
		{
			int32 RawSlotPosition = GetArrayIndex(LoadoutSlot);
			const FResolvedGadgetSlot& ResolvedGadgetSlot = Loadout.ResolvedGadgetSlots[RawSlotPosition];
			return (ResolvedGadgetSlot.ActualType == ECharacterItemType::Weapon) ? ResolvedGadgetSlot.ResolvedArrayIndex : INDEX_NONE;
		}
		
		default: return INDEX_NONE;
	}
}

int32 UWeaponLogicComponent::GetGadgetIndexForSlot(ELoadoutSlot LoadoutSlot)
{
	int32 SlotPosition = GetArrayIndex(LoadoutSlot);
	const FResolvedGadgetSlot& ResolvedGadgetSlot = Loadout.ResolvedGadgetSlots[SlotPosition];
	return (ResolvedGadgetSlot.ActualType == ECharacterItemType::Gadget) ? ResolvedGadgetSlot.ResolvedArrayIndex : INDEX_NONE;
}

ECharacterItemType UWeaponLogicComponent::GetActualGadgetItemType(int32 RawGadgetSlotIndex)
{
	return Loadout.ResolvedGadgetSlots.IsValidIndex(RawGadgetSlotIndex) ? Loadout.ResolvedGadgetSlots[RawGadgetSlotIndex].ActualType : ECharacterItemType::Gadget;
}

FName UWeaponLogicComponent::GetSocketNameForSlot(EAttachmentSlot Slot)
{
	switch (Slot)
	{
		case EAttachmentSlot::FrontSight:
			return TEXT("S_FrontSight");
		case EAttachmentSlot::RearSight:
			return TEXT("S_RearSight");
		case EAttachmentSlot::Scope:       
			return TEXT("S_Optic");
		case EAttachmentSlot::ScopeAccessory:
			return TEXT("S_OpticAccessory");
		case EAttachmentSlot::Handguard:
			return TEXT("S_Handguard");
		case EAttachmentSlot::Muzzle:			
			return TEXT("S_Muzzle");
		case EAttachmentSlot::Underbarrel:		
			return TEXT("S_Underbarrel");
		case EAttachmentSlot::LeftRail:	    
			return TEXT("S_Rail_L");
		case EAttachmentSlot::RightRail:   
			return TEXT("S_Rail_R");
		case EAttachmentSlot::PistolGrip:
			return TEXT("S_PistolGrip");
		case EAttachmentSlot::Magazine:    
			return TEXT("S_Mag");
		case EAttachmentSlot::Stock:       
			return TEXT("S_Stock");
	}
	return NAME_None;
}

FTransform UWeaponLogicComponent::GetSightTransform()
{
	FTransform SightTransform = Loadout.WeaponSystem.InfantryWeaponSystem.WeaponState_FP[GetCII()].WeaponMesh->GetSocketTransform(FName("Aimpoint"), ERelativeTransformSpace::RTS_Component);
	FInfantryWeaponState& InfantryWeaponState = Loadout.WeaponSystem.InfantryWeaponSystem.WeaponState_FP[GetCII()];
	float VerticalAimpointOffset = 0.0f;
	FName AttachmentID = NAME_None;
	if (FWeaponAttachmentState* OpticState = InfantryWeaponState.WeaponAttachmentStates.Find(EAttachmentSlot::Scope))
	{
		AttachmentID = OpticState->BaseAttachmentState.AttachmentID;
	}
	else if (FWeaponAttachmentState* RearSightState = InfantryWeaponState.WeaponAttachmentStates.Find(EAttachmentSlot::RearSight))
	{
		AttachmentID = RearSightState->BaseAttachmentState.AttachmentID;
	}
	if (!AttachmentID.IsNone())
	{
		VerticalAimpointOffset = UBS2FunctionLibrary::GetDataSubsystem(this)->GetWeaponAttachmentDataRow(AttachmentID)->WeaponSightData.VerticalAimpointOffset;
		FVector NewLocation = SightTransform.GetLocation();
		NewLocation.Z += VerticalAimpointOffset;
		SightTransform.SetLocation(NewLocation);
	}
	return SightTransform;
}

float UWeaponLogicComponent::GetSightDistance()
{
	FName& WeaponID = Loadout.WeaponSystem.BaseWeaponSystem.Weapons[GetCII()].WeaponID;
	const float& DefaultSightDistance = UBS2FunctionLibrary::GetDataSubsystem(this)->GetInfantryWeaponDataRow(WeaponID)->InfantryWeaponAimData.DefaultSightDistance;
	FInfantryWeaponState& InfantryWeaponState = Loadout.WeaponSystem.InfantryWeaponSystem.WeaponState_FP[GetCII()];
	FName AttachmentID = NAME_None;
	if (FWeaponAttachmentState* OpticState = InfantryWeaponState.WeaponAttachmentStates.Find(EAttachmentSlot::Scope))
	{
		AttachmentID = OpticState->BaseAttachmentState.AttachmentID;
	}
	else if (FWeaponAttachmentState* RearSightState = InfantryWeaponState.WeaponAttachmentStates.Find(EAttachmentSlot::RearSight))
	{
		AttachmentID = RearSightState->BaseAttachmentState.AttachmentID;
	}
	else
	{
		return DefaultSightDistance;
	}
	const float& SightDistanceOffset = UBS2FunctionLibrary::GetDataSubsystem(this)->GetWeaponAttachmentDataRow(AttachmentID)->WeaponSightData.SightDistanceOffset;
	float FinalSightDistance = DefaultSightDistance + SightDistanceOffset;
	return FinalSightDistance;
}

void UWeaponLogicComponent::GetAimSpeeds(float& AimInSpeed, float& AimOutSpeed)
{
	FName& WeaponID = Loadout.WeaponSystem.BaseWeaponSystem.Weapons[GetCII()].WeaponID;
	const float& DefaultAimInSpeed = UBS2FunctionLibrary::GetDataSubsystem(this)->GetInfantryWeaponDataRow(WeaponID)->InfantryWeaponAimData.DefaultAimInSpeed;
	const float& DefaultAimOutSpeed = UBS2FunctionLibrary::GetDataSubsystem(this)->GetInfantryWeaponDataRow(WeaponID)->InfantryWeaponAimData.DefaultAimOutSpeed;
	TMap<EAttachmentSlot, FWeaponAttachmentState>& WeaponAttachmentStates = Loadout.WeaponSystem.InfantryWeaponSystem.WeaponState_FP[GetCII()].WeaponAttachmentStates;
	TArray<FStatModifierData> ADSModifiers;
	GetAllAttachmentModifierDataOfTypeForWeapon(WeaponAttachmentStates, EWeaponStat::ADSInSpeed, ADSModifiers);
	float CurrentAimInSpeed = CalculateFinalStatValue(DefaultAimInSpeed, ADSModifiers);
	AimInSpeed = CurrentAimInSpeed;
	AimOutSpeed = DefaultAimOutSpeed;
}

void UWeaponLogicComponent::GetAllAttachmentModifierDataOfTypeForWeapon(TMap<EAttachmentSlot, FWeaponAttachmentState>& WeaponAttachmentStates, EWeaponStat WeaponStatType, TArray<FStatModifierData>& OutAttachmentModifierData)
{
	//consider doing this on start or pickup of a weapon and gather/cache a source of truth/data struct for THE CURRENT STATS of the weapon with all of its attachments
	//that way we may not have to do expensive lookups
	for (auto& AttachmentSlot : WeaponAttachmentStates)
	{
		FWeaponAttachmentState& AttachmentState = AttachmentSlot.Value;
		FName& AttachmentID = AttachmentState.BaseAttachmentState.AttachmentID;
		const FStatModifierData* NewAttachmentModifierData = UBS2FunctionLibrary::GetDataSubsystem(this)->GetWeaponAttachmentDataRow(AttachmentID)->AttachmentModifiers.Find(WeaponStatType);
		if (NewAttachmentModifierData)
		{
			OutAttachmentModifierData.Add(*NewAttachmentModifierData);
		}
	}
}

FWeaponAttachmentState& UWeaponLogicComponent::GetCurrentAttachmentInSlot(EAttachmentSlot Slot)
{
	if (Loadout.WeaponSystem.InfantryWeaponSystem.WeaponState_FP[GetCII()].WeaponAttachmentStates.Contains(Slot))
	{
		return Loadout.WeaponSystem.InfantryWeaponSystem.WeaponState_FP[GetCII()].WeaponAttachmentStates[Slot];
	}
	else
	{
		static FWeaponAttachmentState DefaultAttachmentState;
		return DefaultAttachmentState;
	}
}

FWeapon_Runtime& UWeaponLogicComponent::GetBaseWeaponState(int32 WeaponIndex)
{
	return Loadout.WeaponSystem.BaseWeaponSystem.Weapons[WeaponIndex];
}

FInfantryWeaponState& UWeaponLogicComponent::GetCurrentInfantryWeaponState_FP()
{
	return Loadout.WeaponSystem.InfantryWeaponSystem.WeaponState_FP[GetCII()];
}

FVector UWeaponLogicComponent::GetAttachmentDefaultOffset(FName WeaponID, EAttachmentSlot Slot, FName AttachmentID)
{
	return UBS2FunctionLibrary::GetDataSubsystem(this)->GetInfantryWeaponDataRow(WeaponID)->GunAttachmentData.AvailableAttachmentSlots.Find(Slot)->Attachments.Find(AttachmentID)->LocationOffset;
}

FWeapon_Runtime* UWeaponLogicComponent::GetCurrentWeaponRuntime()
{
	FWeapon_Runtime* CurrentWeapon = &Loadout.WeaponSystem.BaseWeaponSystem.Weapons[GetCII()];
	return CurrentWeapon;
}

const FInfantryWeaponData* UWeaponLogicComponent::GetCurrentWeaponStaticData() 
{
	//assumes currentitemindex is correct and that current slot is weapon
	const int32 WeaponIndex = GetCII();
	const FInfantryWeaponData* StaticWeaponData = StaticWeaponDataCache[WeaponIndex];
	return StaticWeaponData;
}

FInfantryWeaponData UWeaponLogicComponent::GetCurrentWeaponStaticData_BP()
{
	const FInfantryWeaponData* StaticWeaponData = GetCurrentWeaponStaticData();
	return *StaticWeaponData;
}

FWeaponStats_Runtime& UWeaponLogicComponent::GetCurrentWeaponStats()
{
	return Loadout.WeaponSystem.InfantryWeaponSystem.CurrentWeaponStats[GetCII()];
}

int32 UWeaponLogicComponent::GetCII()
{
	int32 ItemIndex = INDEX_NONE;
	ECharacterItemType CurrentItemType = GetCategoryForSlot(Loadout.CurrentSlot);
	if (CurrentItemType == ECharacterItemType::Gadget)
	{
		CurrentItemType = GetActualGadgetItemType(GetArrayIndex(Loadout.CurrentSlot));
	}
	
	switch (CurrentItemType)
	{
		case ECharacterItemType::Weapon:
			ItemIndex = GetWeaponIndexForSlot(Loadout.CurrentSlot);
			break;
		case ECharacterItemType::Gadget:
			ItemIndex = GetGadgetIndexForSlot(Loadout.CurrentSlot);
			break;
		default:
			ItemIndex = INDEX_NONE;
			break;
	}
	
	return ItemIndex;
}

ACharacter_Base* UWeaponLogicComponent::GetOwnerCharacter()
{
	return Cast<ACharacter_Base>(GetOwner());
}

#pragma endregion

