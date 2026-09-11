
#include "Core/Weapons/LoadoutManager.h"
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
#include "Utilities/I_Anims.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Utilities/HUDSubsystem.h"

ULoadoutManager::ULoadoutManager()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void ULoadoutManager::BeginPlay()
{
	Super::BeginPlay();
}

void ULoadoutManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	Rangefinder();
}

#pragma region Initialization/Factory

void ULoadoutManager::Init_Loadout(FPlayerLoadoutConfig_Class ClassLoadout)
{
	TArray<FName> WeaponIDs = ClassLoadout.Weapons;
	TArray<FPlayerLoadoutConfig_Weapon> FinalWeaponLoadouts = ClassLoadout.WeaponLoadouts;
	TArray<FName> GadgetIDs;
	Loadout.ResolvedGadgetSlots.SetNum(ClassLoadout.Gadgets.Num());
	
	while (FinalWeaponLoadouts.Num() < WeaponIDs.Num())
	{
		FinalWeaponLoadouts.Add(FPlayerLoadoutConfig_Weapon());
	}
	
	//INIT_ResolveWeaponGadgets
	//add any gadgets that are weapons to the weaponID list for weapon initialization
	for (int32 i = 0; i < ClassLoadout.Gadgets.Num(); i++)
	{
		const FGadgetData* NewGadgetData = UBS2FunctionLibrary::GetDataSubsystem(this)->GetGadgetDataRow(ClassLoadout.Gadgets[i]);
		if (NewGadgetData)
		{
			Loadout.ResolvedGadgetSlots[i].ActualType = ECharacterItemType::Gadget;
			Loadout.ResolvedGadgetSlots[i].ResolvedArrayIndex = GadgetIDs.Num();
			GadgetIDs.Add(ClassLoadout.Gadgets[i]);
			continue;
		}
		
		//if a data row isn't found given the id, it might actually be a weapon
		//if the gadgetid is in fact a valid infantry weapon data row, remove the id from the gadget list, add it to the weapon list.
		const FInfantryWeaponData* InfantryWeaponData = UBS2FunctionLibrary::GetDataSubsystem(this)->GetInfantryWeaponDataRow(ClassLoadout.Gadgets[i]);
		if (InfantryWeaponData)
		{
			Loadout.ResolvedGadgetSlots[i].ActualType = ECharacterItemType::Weapon;
			Loadout.ResolvedGadgetSlots[i].ResolvedArrayIndex = WeaponIDs.Num();
			WeaponIDs.Add(ClassLoadout.Gadgets[i]);
			
			FPlayerLoadoutConfig_Weapon GadgetWeaponConfig = ClassLoadout.GadgetWeaponLoadouts.IsValidIndex(i)? ClassLoadout.GadgetWeaponLoadouts[i] : FPlayerLoadoutConfig_Weapon();
			
			//??? meant to apply weapon attachments in weaponloadout list to weapon gadgets, possible bugs
			//if (!FinalWeaponLoadouts.IsValidIndex(i))
			//{
			FinalWeaponLoadouts.Add(GadgetWeaponConfig); 
			//}
		}
	}
	
	Init_WeaponLoadout(WeaponIDs, FinalWeaponLoadouts);
	Init_GadgetLoadout(GadgetIDs);
	
	isInitialized = true;
}

void ULoadoutManager::Init_WeaponLoadout(TArray<FName> Weapons, TArray<FPlayerLoadoutConfig_Weapon> WeaponLoadouts)
{
	Loadout.WeaponSystem.BaseWeaponState.Weapons.SetNum(Weapons.Num());
	StaticWeaponDataCache.SetNum(Weapons.Num());
	Loadout.WeaponSystem.InfantryWeaponState.WeaponState_FP.SetNum(Weapons.Num());
	Loadout.WeaponSystem.InfantryWeaponState.WeaponState_TP.SetNum(Weapons.Num());
	Loadout.WeaponSystem.InfantryWeaponState.CurrentWeaponStats.SetNum(Weapons.Num());

	for (int32 i = 0; i < Weapons.Num(); i++)
	{
		FPlayerLoadoutConfig_Weapon LoadoutConfig = WeaponLoadouts.IsValidIndex(i)? WeaponLoadouts[i] : FPlayerLoadoutConfig_Weapon();
		Init_Weapon(Weapons[i], i, LoadoutConfig);
	}
	Init_WAC();
	Init_ScopeCamera();
	EquipWeapon(0, true);
}

void ULoadoutManager::Init_GadgetLoadout(TArray<FName> Gadgets)
{
	Loadout.Gadgets.SetNum(Gadgets.Num());
	StaticGadgetDataCache.SetNum(Gadgets.Num());
	
	for (int32 i = 0; i < Gadgets.Num(); i++)
	{
		Init_Gadget(Gadgets[i], i);
	}
}

void ULoadoutManager::Init_ScopeCamera()
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

void ULoadoutManager::Init_Weapon(FName WeaponID, int32 WeaponIndex, FPlayerLoadoutConfig_Weapon WeaponLoadout)
{
	//Init WeaponSlot		Init Weapon
	FInfantryWeaponState NewFPState;
	Init_WeaponMesh(NewFPState.WeaponMesh);
	NewFPState.WeaponMesh->SetOnlyOwnerSee(true);
	UpdateWeaponMesh(WeaponID, NewFPState.WeaponMesh);
	NewFPState.WeaponMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	Loadout.WeaponSystem.InfantryWeaponState.WeaponState_FP[WeaponIndex] = NewFPState;
	
	FString SocketString = FString::Printf(TEXT("Socket_%s"), *WeaponID.ToString());
	FName AttachSocketName = FName(*SocketString);
	Loadout.WeaponSystem.InfantryWeaponState.WeaponState_FP[WeaponIndex].WeaponMesh->AttachToComponent(Cast<ACharacter_Base>(GetOwner())->FPArms, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, true), AttachSocketName);
	
	Loadout.WeaponSystem.BaseWeaponState.Weapons[WeaponIndex].WeaponID = WeaponID;
	check (UBS2FunctionLibrary::GetDataSubsystem(this)->GetInfantryWeaponDataRow(WeaponID));
	StaticWeaponDataCache[WeaponIndex] = UBS2FunctionLibrary::GetDataSubsystem(this)->GetInfantryWeaponDataRow(WeaponID);
	
	SetupCustomWeapon(WeaponIndex, WeaponLoadout);
	
	Init_WeaponState(WeaponIndex);
	UpdateWeaponVisibility(WeaponIndex, true);
}

void ULoadoutManager::Init_WeaponMesh(TWeakObjectPtr<USkeletalMeshComponent>& WeaponMesh)
{
	TWeakObjectPtr<USkeletalMeshComponent> NewWeapon = NewObject<USkeletalMeshComponent>(GetOwner());
	NewWeapon->RegisterComponent();
	WeaponMesh = NewWeapon;			//cache
}

void ULoadoutManager::Init_WeaponState(int32 WeaponIndex)
{
	//if using current weapon stats to initialize, the stats need to be valid/setup beforehand
	FWeaponStats_Runtime& CurrentWeaponStats = Loadout.WeaponSystem.InfantryWeaponState.CurrentWeaponStats[WeaponIndex];
	FWeaponState& BaseWeaponState = GetBaseWeaponState(WeaponIndex);
	BaseWeaponState.CurrentAmmoinMag = GetMaxMagSize(WeaponIndex);
	BaseWeaponState.CurrentReserveAmmo = CurrentWeaponStats.MaxReserveAmmo;
	BaseWeaponState.CurrentFireMode = CurrentWeaponStats.FireModeData.DefaultFireMode;
}

void ULoadoutManager::Init_WAC()
{
	Loadout.WeaponSystem.WeaponAudioComponent = UBS2FunctionLibrary::CreateWAC(this, GetOwner(), GetOwner()->GetRootComponent());
}

void ULoadoutManager::Init_Attachment(FWeaponAttachmentState& RuntimeSlotState, FInfantryWeaponState& WeaponToApplyTo, EAttachmentSlot AttachmentSlot)
{
	//Initialize/Create Attachment, attach to gun, cache
	TWeakObjectPtr<UStaticMeshComponent> NewAttachment = NewObject<UStaticMeshComponent>(GetOwner());
	NewAttachment->RegisterComponent();
	NewAttachment->AttachToComponent(WeaponToApplyTo.WeaponMesh.Get(), FAttachmentTransformRules::SnapToTargetIncludingScale, GetSocketNameForSlot(AttachmentSlot));
	RuntimeSlotState.SpawnedAttachment = NewAttachment;
}

void ULoadoutManager::Init_Gadget(FName GadgetID, int32 GadgetIndex)
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

void ULoadoutManager::Init_GadgetMesh(TWeakObjectPtr<UStaticMeshComponent>& HeldGadgetMesh)
{
	TWeakObjectPtr<UStaticMeshComponent> NewGadgetMesh = NewObject<UStaticMeshComponent>(GetOwner());
	NewGadgetMesh->RegisterComponent();
	HeldGadgetMesh = NewGadgetMesh;
}

#pragma endregion

void ULoadoutManager::SetupCustomWeapon(int32 WeaponIndex, FPlayerLoadoutConfig_Weapon WeaponLoadout)
{
	//setup custom weapon (attachments, stats, etc)
	const FPlayerLoadoutConfig_Weapon& CustomWeapon = WeaponLoadout;
	ApplyAttachments(CustomWeapon, WeaponIndex);
	UpdateCurrentWeaponStats(WeaponIndex);
}

void ULoadoutManager::ApplyAttachments(const FPlayerLoadoutConfig_Weapon& AttachmentsToApply, int32 WeaponIndex)
{
	FInfantryWeaponState& WeaponToApplyTo = Loadout.WeaponSystem.InfantryWeaponState.WeaponState_FP[WeaponIndex];
	for (auto& Slot : AttachmentsToApply.WeaponAttachments)
	{
		const EAttachmentSlot& SlotType = Slot.Key;
		const FPlayerLoadoutConfig_WeaponAttachment& AttachmentConfig = Slot.Value;
		FWeaponAttachmentState& RuntimeSlotState = WeaponToApplyTo.WeaponAttachmentStates.FindOrAdd(SlotType);

		Init_Attachment(RuntimeSlotState, WeaponToApplyTo, SlotType);
		UpdateAttachment(RuntimeSlotState, AttachmentConfig.AttachmentID, GetBaseWeaponState(WeaponIndex).WeaponID, SlotType);
	}
}

void ULoadoutManager::UpdateWeaponMesh(FName WeaponID, TWeakObjectPtr<USkeletalMeshComponent>& WeaponMeshComp)
{
	const FInfantryWeaponData& WeaponData = *UBS2FunctionLibrary::GetDataSubsystem(this)->GetInfantryWeaponDataRow(WeaponID);
	TWeakObjectPtr<USkeletalMesh> WeaponMesh = WeaponData.WeaponClassificationData.WeaponMesh.LoadSynchronous();
	WeaponMeshComp->SetSkeletalMesh(WeaponMesh.Get());
}

void ULoadoutManager::UpdateGadgetMesh(FName GadgetID, TWeakObjectPtr<UStaticMeshComponent>& GadgetMeshComp)
{
	const FGadgetData& GadgetData = *UBS2FunctionLibrary::GetDataSubsystem(this)->GetGadgetDataRow(GadgetID);
	TWeakObjectPtr<UStaticMesh> GadgetMesh = GadgetData.GadgetMesh.LoadSynchronous();
	GadgetMeshComp->SetStaticMesh(GadgetMesh.Get());
}

void ULoadoutManager::UpdateGadgetVisibility(int32 GadgetIndex, bool Hide)
{
	Loadout.Gadgets[GadgetIndex].HeldMesh_FP->SetHiddenInGame(Hide);
}

void ULoadoutManager::UpdateGadgetCollision(ECollisionChannel CollisionChannel, ECollisionResponse CollisionResponse, int32 GadgetIndex)
{
	Loadout.Gadgets[GadgetIndex].HeldMesh_FP.Get()->SetCollisionResponseToChannel(CollisionChannel, CollisionResponse);
}

void ULoadoutManager::UpdateWeaponData(int32 WeaponIndex, FName WeaponID, FInfantryWeaponState WeaponState)
{
	Loadout.WeaponSystem.BaseWeaponState.Weapons[WeaponIndex].WeaponID = WeaponID;
	StaticWeaponDataCache[WeaponIndex] = UBS2FunctionLibrary::GetDataSubsystem(this)->GetInfantryWeaponDataRow(WeaponID);

	//add a bool or an int if it should fp, tp or both
	Loadout.WeaponSystem.InfantryWeaponState.WeaponState_FP[WeaponIndex] = WeaponState;
}

void ULoadoutManager::UpdateScopeCamera()
{
	//called on equip weapon
	Loadout.WeaponSystem.ScopeCamera->AttachToComponent(Loadout.WeaponSystem.InfantryWeaponState.WeaponState_FP[GetCII()].WeaponMesh.Get(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, true), FName("PIP"));

	if (Loadout.WeaponSystem.InfantryWeaponState.WeaponState_FP[GetCII()].WeaponAttachmentStates.Find(EAttachmentSlot::Scope))
	{
		FWeaponAttachmentState& WeaponAttachmentState = *Loadout.WeaponSystem.InfantryWeaponState.WeaponState_FP[GetCII()].WeaponAttachmentStates.Find(EAttachmentSlot::Scope);
		int32 PIPMatIndex = UBS2FunctionLibrary::GetDataSubsystem(this)->GetWeaponAttachmentDataRow(WeaponAttachmentState.BaseAttachmentState.AttachmentID)->WeaponSightData.PIPMaterialIndex;
		if (PIPMatIndex < 0) { return; }
		UMaterialInstanceDynamic* MID = WeaponAttachmentState.SpawnedAttachment->CreateDynamicMaterialInstance(PIPMatIndex, WeaponAttachmentState.SpawnedAttachment->GetMaterial(PIPMatIndex));
		WeaponAttachmentState.SpawnedAttachment->SetMaterial(PIPMatIndex, MID);
		MID->SetTextureParameterValue(FName("RenderTarget"), Loadout.WeaponSystem.ScopeCamera->TextureTarget);
	}
}

void ULoadoutManager::UpdateAttachment(FWeaponAttachmentState& RuntimeSlotState, FName AttachmentID, FName WeaponID, EAttachmentSlot AttachmentSlot)
{
	//update attachment mesh
	const FWeaponAttachmentData& AttachmentData = *UBS2FunctionLibrary::GetDataSubsystem(this)->GetWeaponAttachmentDataRow(AttachmentID);
	TWeakObjectPtr<UStaticMesh> AttachmentMesh = AttachmentData.AttachmentClassification.AttachmentMesh.LoadSynchronous();
	const FVector& AttachmentOffset = GetAttachmentDefaultOffset(WeaponID, AttachmentSlot, AttachmentID);

	RuntimeSlotState.SpawnedAttachment->SetStaticMesh(AttachmentMesh.Get());
	RuntimeSlotState.SpawnedAttachment->SetRelativeLocation(AttachmentOffset);
	RuntimeSlotState.BaseAttachmentState.AttachmentID = AttachmentID;
}

void ULoadoutManager::UpdateWeaponCollision(ECollisionChannel CollisionChannel, ECollisionResponse CollisionResponse, int32 WeaponIndex)
{
	Loadout.WeaponSystem.InfantryWeaponState.WeaponState_FP[WeaponIndex].WeaponMesh->SetCollisionResponseToChannel(CollisionChannel, CollisionResponse);
	for (auto& Attachment : Loadout.WeaponSystem.InfantryWeaponState.WeaponState_FP[WeaponIndex].WeaponAttachmentStates)
	{
		Attachment.Value.SpawnedAttachment.Get()->SetCollisionResponseToChannel(CollisionChannel, CollisionResponse);
	}
	//WeaponSystem.InfantryWeaponSystem.WeaponState_TP[WeaponSystem.BaseWeaponSystem.EquippedWeaponState.CurrentWeaponIndex].WeaponMesh->SetCollisionResponseToChannel(CollisionChannel, CollisionResponse);
}

void ULoadoutManager::UpdateWeaponVisibility(int32 WeaponIndex, bool Hide)
{
	FInfantryWeaponState& Weapon = Loadout.WeaponSystem.InfantryWeaponState.WeaponState_FP[WeaponIndex];
	Weapon.WeaponMesh->SetHiddenInGame(Hide);
	for (auto& AttachmentSlot : Weapon.WeaponAttachmentStates)
	{
		AttachmentSlot.Value.SpawnedAttachment->SetHiddenInGame(Hide);
	}
}

#pragma region Aiming

void ULoadoutManager::StartAim()
{
	if (!GetIsCurrentSlotActuallyWeapon())	{ return; }
	if (!CombatState.canAim)
	{
		if (CombatState.isAiming)
		{
			StopAim();
		}
		return;
	}
	const FInfantryWeaponAimData& AimData = GetCurrentWeaponStaticData()->InfantryWeaponAimData;
	CombatState.canAim = AimData.canAim;

	
	if (AimData.HideArms)
	{
		TWeakObjectPtr<ACharacter_Base> Character = Cast<ACharacter_Base>(GetOwner());
		USkeletalMeshComponent* FPArms = Character->FPArms;
		FPArms->SetVisibility(false);
		FString SocketString = FString::Printf(TEXT("Socket_%s_1"), *GetCurrentWeaponBaseState()->WeaponID.ToString());
		FName AttachSocketName = FName(*SocketString);
		if (FPArms->DoesSocketExist(AttachSocketName))
		{
			GetCurrentInfantryWeaponState_FP().WeaponMesh->AttachToComponent(Character->FPArms, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, true), AttachSocketName);
		}
	}
	CombatState.isAiming = true;
}

void ULoadoutManager::StopAim()
{
	const FInfantryWeaponAimData& AimData = GetCurrentWeaponStaticData()->InfantryWeaponAimData;
	if (!AimData.canAim) { return; }
	if (!CombatState.isAiming) { return; }
	if (AimData.HideArms)
	{
		TWeakObjectPtr<ACharacter_Base> Character = Cast<ACharacter_Base>(GetOwner());
		USkeletalMeshComponent* FPArms = Character->FPArms;
		FPArms->SetVisibility(true);
		FString SocketString = FString::Printf(TEXT("Socket_%s"), *GetCurrentWeaponBaseState()->WeaponID.ToString());
		FName AttachSocketName = FName(*SocketString);
		GetCurrentInfantryWeaponState_FP().WeaponMesh->AttachToComponent(Character->FPArms, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, true), AttachSocketName);
	}
	CombatState.isAiming = false;
}

# pragma endregion 

void ULoadoutManager::Rangefinder()
{
	FHitResult OutHit;
	UBS2FunctionLibrary::PerformWeaponLineTrace(this, GetOwnerCharacter()->FPCamera->GetComponentTransform(), OutHit, { GetOwner() }, false);
	if (Loadout.WeaponSystem.InfantryWeaponState.WeaponState_FP.IsEmpty())	{ return;}
	TWeakObjectPtr<USkeletalMeshComponent>& WeaponMesh = Loadout.WeaponSystem.InfantryWeaponState.WeaponState_FP[GetCII()].WeaponMesh;

	Loadout.WeaponSystem.BaseWeaponState.EquippedWeaponState.RaycastData.RangefinderData = OutHit;

	Loadout.WeaponSystem.BaseWeaponState.EquippedWeaponState.RaycastData.MuzzleAimDirections[0] = UBS2FunctionLibrary::GetAimDirectionFromMuzzle(OutHit, FName("Muzzle"), WeaponMesh);
}

#pragma region WeaponFire

void ULoadoutManager::HandleStartFire()
{
	FWeaponState& CurrentWeapon = *GetCurrentWeaponBaseState();

 	if (!CurrentWeapon.canFire)
	{
		if (CurrentWeapon.CurrentAmmoinMag <= 0 && !CombatState.isAttemptingToFire)
		{
			CombatState.isAttemptingToFire = true;
			DryFire();
		}
		return;
	}
	
  	switch (CurrentWeapon.CurrentFireMode)
	{
		case EFireMode::Single:
			if (!CombatState.isAttemptingToFire)
			{
				CombatState.isAttemptingToFire = true;
				StartFire();
			}
			break;
		case EFireMode::Burst:
			break;
		case EFireMode::Auto:
  			if (!GetWorld()->GetTimerManager().IsTimerActive(CurrentWeapon.TimerHandle_AutoFire))
  			{
  				CombatState.isAttemptingToFire = true;
  				StartFire();
				StartAutoFire();
  			}
			break;
	}
}

void ULoadoutManager::StartFire()
{
	//fires exactly once
	//assumes canfire is true
	UBS2FunctionLibrary::StartWAC(Loadout.WeaponSystem.WeaponAudioComponent);
	
	GetCurrentWeaponBaseState()->isFiring = true;
	
	if (GetOwnerCharacter()->CharacterState.CharacterMovementState.CurrentMovementMode == ECharacterMovementMode::Sprinting)
	{
		GetOwnerCharacter()->UpdateMovementMode(ECharacterMovementMode::Walking);
	}
	GetOwnerCharacter()->CharacterState.CharacterMovementState.canSprint = false;
	
	FireWeapon();
}

void ULoadoutManager::StartAutoFire()
{
	FWeaponState& CurrentWeapon = *GetCurrentWeaponBaseState();

	if (CurrentWeapon.canFire)	//if here so if the first fire changed this state
	{
		float FireRate = UBS2FunctionLibrary::GetFireRate(GetCurrentWeaponStaticData()->WeaponFirePerformanceData.RateOfFire);

		// Create a Weak Lambda
		FTimerDelegate FireDelegate;
		FireDelegate.BindWeakLambda(this, [this]()
		{
			// This code ONLY runs if 'this' is still a valid, non-null pointer
			this->FireWeapon();
		});

		GetWorld()->GetTimerManager().SetTimer(CurrentWeapon.TimerHandle_AutoFire, FireDelegate, FireRate, true);
	}
}

void ULoadoutManager::ShootSimProjectile()
{
	FVector MuzzleLocation = UBS2FunctionLibrary::GetMuzzleTransform(FName("Muzzle"), GetCurrentInfantryWeaponState_FP().WeaponMesh).GetLocation();
	
	const FInfantryWeaponData& StaticWeaponData = *GetCurrentWeaponStaticData();
	FWeaponState& WeaponState = *GetCurrentWeaponBaseState();
	FEquippedWeaponState& EWS = Loadout.WeaponSystem.BaseWeaponState.EquippedWeaponState;
	UBS2FunctionLibrary::CreateSimProjectile
	(
		StaticWeaponData.WeaponFirePerformanceData.MunitionID,
		nullptr,
		MuzzleLocation,
		StaticWeaponData.WeaponFirePerformanceData.MuzzleVelocity,
		StaticWeaponData.WeaponFirePerformanceData.GravityScale,
		EWS.RaycastData.MuzzleAimDirections[0],
		StaticWeaponData.WeaponFirePerformanceData.WeaponDamageData.BaseDamage,
		StaticWeaponData.WeaponFirePerformanceData.WeaponDamageData.DamageDropoffCurve,
		UBS2FunctionLibrary::GetProjectileSystem(this)
	);
}

void ULoadoutManager::HandleShootProjectileActor()
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
		FVector& AimDirection = Loadout.WeaponSystem.BaseWeaponState.EquippedWeaponState.RaycastData.MuzzleAimDirections[0];
		FTransform MuzzleTransform = UBS2FunctionLibrary::GetMuzzleTransform(FName("Muzzle"), GetCurrentInfantryWeaponState_FP().WeaponMesh);
		FiredProjectile->SetActorTransform(MuzzleTransform);
		FiredProjectile->FireProjectile(AimDirection);
	}
}

#pragma region WeaponRecoil

void ULoadoutManager::TriggerControllerRecoil()
{
	CombatState.RecoilElapsedTime = 0.0f;
	
	FTimerDelegate RecoilDelgate;
	RecoilDelgate.BindWeakLambda(this, [this]()
	{
		UpdateControllerRecoil();
	});
	
	GetWorld()->GetTimerManager().SetTimer(CombatState.TimerHandle_Recoil, RecoilDelgate, 1.0f/60.0f, true);
}

void ULoadoutManager::UpdateControllerRecoil()
{
	const FInfantryWeaponData& StaticWeaponData = *GetCurrentWeaponStaticData();
	const FControllerRecoilData& RecoilData = StaticWeaponData.WeaponRecoilData.ControllerRecoilData;
	TObjectPtr<ACharacter_Base> Character = GetOwnerCharacter();
	
	float PitchValue = RecoilData.PitchControllerRecoil->GetFloatValue(CombatState.RecoilElapsedTime);
	float YawValue = RecoilData.YawControllerRecoil->GetFloatValue(CombatState.RecoilElapsedTime);
	
	if (!CombatState.isAiming)
	{
		PitchValue *= RecoilData.HipfireControllerRecoilMultiplier;
		YawValue *= RecoilData.HipfireControllerRecoilMultiplier;
	}
	
	Character->AddControllerPitchInput(PitchValue);
	Character->AddControllerYawInput(YawValue);
	
	CombatState.RecoilElapsedTime += 1.0f/60.0f;	
	
	if (CombatState.RecoilElapsedTime >= RecoilData.PitchControllerRecoil->FloatCurve.GetLastKey().Time)
	{
		GetWorld()->GetTimerManager().ClearTimer(CombatState.TimerHandle_Recoil);
	}
}

#pragma endregion

void ULoadoutManager::CeaseFire()
{
	//just because fire is ceased, it SHOULD NOT be assumed that weapon cannot fire
	CombatState.isAttemptingToFire = false;
	FWeaponState& CurrentWeapon = *GetCurrentWeaponBaseState();
	
	Loadout.WeaponSystem.WeaponAudioComponent->SetTriggerParameter(FName("Event_StopFire"));
	Loadout.WeaponSystem.WeaponAudioComponent->OnAudioFinishedNative.AddWeakLambda(this, [this](UAudioComponent* FinishedComponent)
	{
		FinishedComponent->Deactivate();
	});
	
	if (GetWorld()->GetTimerManager().IsTimerActive(CurrentWeapon.TimerHandle_AutoFire))
	{
		GetWorld()->GetTimerManager().ClearTimer(CurrentWeapon.TimerHandle_AutoFire);
	}

	CurrentWeapon.isFiring = false;
	GetOwnerCharacter()->CharacterState.CharacterMovementState.canSprint = false;
}

void ULoadoutManager::DryFire()
{
	//play whatever dryfire sound and animation
	FWeaponState& CurrentWeapon = *GetCurrentWeaponBaseState();

	if (CurrentWeapon.canFire)
	{
		CurrentWeapon.canFire = false;
	}

	UGameplayStatics::PlaySound2D(this,GetCurrentWeaponStaticData()->InfantryWeaponAudioData.DryFireSFX.LoadSynchronous());
}

void ULoadoutManager::FireWeapon()
{
	const FInfantryWeaponData* StaticWeaponData = GetCurrentWeaponStaticData();
	
	FWeaponState& CurrentWeapon = *GetCurrentWeaponBaseState();	
	FInfantryWeaponState& IWS_FP = GetCurrentInfantryWeaponState_FP();

	switch(StaticWeaponData->WeaponFirePerformanceData.WeaponFireType)
	{
		case EWeaponFireType::SimProjectile:
			ShootSimProjectile();
			break;
		case EWeaponFireType::ActorProjectile:
			HandleShootProjectileActor();
			break;
		case EWeaponFireType::VFX:
			break;
		case EWeaponFireType::Hitscan:
			break;
	}
	
	//RECOIL
	TriggerControllerRecoil();
	IAnims::Execute_IKRecoil(GetOwnerCharacter()->FPArms->GetAnimInstance(), StaticWeaponData->WeaponRecoilData.IKProceduralRecoilData);
	
	//TRIGGER EFFECTS
	if (StaticWeaponData->WeaponVFXData.MuzzleSmokeParticle)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetOwner(), StaticWeaponData->WeaponVFXData.MuzzleSmokeParticle.LoadSynchronous(), IWS_FP.WeaponMesh->GetSocketLocation(FName("Muzzle")), IWS_FP.WeaponMesh->GetSocketRotation(FName("Muzzle")), FVector::ZeroVector, true, true, ENCPoolMethod::None, true);
	}
	if (StaticWeaponData->WeaponVFXData.MuzzleFlashFX)
	{
		UNiagaraComponent* MuzzleFlash = UNiagaraFunctionLibrary::SpawnSystemAttached(StaticWeaponData->WeaponVFXData.MuzzleFlashFX.LoadSynchronous(), IWS_FP.WeaponMesh.Get(), FName("Muzzle"), FVector::ZeroVector, IWS_FP.WeaponMesh->GetSocketRotation(FName("Muzzle")), EAttachLocation::KeepRelativeOffset, false, true, ENCPoolMethod::None, true);
		MuzzleFlash->SetNiagaraVariableBool("User.Trigger", true);
	}
	IWS_FP.WeaponMesh->PlayAnimation(StaticWeaponData->InfantryWeaponAnimData.WeaponAnimData.WeaponFire.LoadSynchronous(), false);
	//weapon fire anim
	//weapon slide anim (third person)
	//weapon fire audio
	//spawn casing
	
	switch (StaticWeaponData->InfantryWeaponAmmoData.BaseAmmoData.AmmoDepletionMethod)
	{
		case EAmmoDepletionMethod::Default:
			UBS2FunctionLibrary::UpdateCurrentAmmoInMag(CurrentWeapon, -1, CurrentWeapon.CurrentAmmoinMag);
			if (GetOwnerCharacter()->IsLocallyControlled())
			{
				UBS2FunctionLibrary::GetHUDSubsystem(this)->UpdateStatusHUD_CAMCount(CurrentWeapon.CurrentAmmoinMag);
			}
			if (CurrentWeapon.CurrentAmmoinMag <= 0)
			{
				CeaseFire();
				DryFire();
			}
			break;
		case EAmmoDepletionMethod::Heat:
			break;
		case EAmmoDepletionMethod::None:
			break;
	}

}

#pragma endregion

#pragma region WeaponReload

void ULoadoutManager::ReloadWeapon()
{
	int32 WeaponIndex = GetCII();
	FWeaponState& CurrentWeapon = *GetCurrentWeaponBaseState();
	const FInfantryWeaponData* StaticWeaponData = GetCurrentWeaponStaticData();
	FInfantryWeaponState& InfantryWeaponState_FP = Loadout.WeaponSystem.InfantryWeaponState.WeaponState_FP[WeaponIndex];
	ReloadEndedDelegate.BindWeakLambda(this, [this, WeaponIndex](UAnimMontage* Montage, bool bInterrupted)
	{
		OnReloadFinished(Montage, bInterrupted, WeaponIndex);
	});
	
	if (!UBS2FunctionLibrary::GetIfWeaponCanReload(CurrentWeapon, StaticWeaponData->InfantryWeaponAmmoData.bCanRoundBeChambered, StaticWeaponData->InfantryWeaponAmmoData.BaseAmmoData.MagSize))		{ return; }
	if (!StaticWeaponData->WeaponFunctionalityData.canADSReload)
	{
		CombatState.canAim = false;
		if (CombatState.isAiming)
		{
			StopAim();		
		}
	}

	CurrentWeapon.isReloading = true;
	if (CurrentWeapon.isFiring)
	{
		CeaseFire();
	}
	CurrentWeapon.canFire = false;
	
	bool bEmptyMag = CurrentWeapon.CurrentAmmoinMag <= 0;

	if (!StaticWeaponData->InfantryWeaponAmmoData.ProjectileBoneToHide.IsNone())
	{
		InfantryWeaponState_FP.WeaponMesh->UnHideBoneByName(StaticWeaponData->InfantryWeaponAmmoData.ProjectileBoneToHide);
	}

	TWeakObjectPtr<ACharacter_Base> Character = GetOwnerCharacter();
	USkeletalMeshComponent* FPArms = Character->FPArms;
	TWeakObjectPtr<UAnimInstance> FPArmsAnimInstance = FPArms->GetAnimInstance();
	float& ReloadSpeed = GetCurrentWeaponStats().ReloadSpeed;

	TSoftObjectPtr<UAnimMontage> FPReloadMontage = bEmptyMag ? StaticWeaponData->InfantryWeaponAnimData.FPWeaponAnimData.ReloadEmptyWeaponMontage : StaticWeaponData->InfantryWeaponAnimData.FPWeaponAnimData.ReloadWeaponMontage;
	TSoftObjectPtr<UAnimSequence> WeaponReloadAnim = bEmptyMag ? StaticWeaponData->InfantryWeaponAnimData.WeaponAnimData.WeaponReloadEmpty : StaticWeaponData->InfantryWeaponAnimData.WeaponAnimData.WeaponReload;
	FPReloadMontage.LoadSynchronous();
	WeaponReloadAnim.LoadSynchronous();

	//InfantryWeaponState_FP.WeaponMesh->PlayAnimation(WeaponReloadAnim.Get(), false);
	UBS2FunctionLibrary::PlayAnimSequenceAtDesiredDuration(InfantryWeaponState_FP.WeaponMesh.Get(), WeaponReloadAnim.Get(), ReloadSpeed, false);
	
	/**
	if (StaticWeaponData->WeaponFunctionalityData.canADSReload && CombatState.isAiming)
	{
		OnReloadFinished(nullptr, false, WeaponIndex);
		return;
	}
	**/
	
	//FPArmsAnimInstance->Montage_Play(FPReloadMontage.Get(), 1.0f);
	UBS2FunctionLibrary::PlayAnimMontageAtDesiredDuration(FPArmsAnimInstance.Get(), FPReloadMontage.Get(), ReloadSpeed);
	FPArmsAnimInstance->Montage_SetEndDelegate(ReloadEndedDelegate, FPReloadMontage.Get());
}

void ULoadoutManager::OnReloadFinished(UAnimMontage* Montage, bool bInterrupted, int32 WeaponIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("[WLC::OnReloadFinished"));
	FWeaponStats_Runtime& CurrentWeaponStats = Loadout.WeaponSystem.InfantryWeaponState.CurrentWeaponStats[WeaponIndex];
	FWeaponState& CurrentWeapon = Loadout.WeaponSystem.BaseWeaponState.Weapons[WeaponIndex];
	const FInfantryWeaponData* StaticWeaponData = StaticWeaponDataCache[WeaponIndex];
	FInfantryWeaponState& InfantryWeaponState_FP = Loadout.WeaponSystem.InfantryWeaponState.WeaponState_FP[WeaponIndex];

	TWeakObjectPtr<ACharacter_Base> Character = GetOwnerCharacter();
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
	
	CurrentWeapon.isReloading = false;
	CurrentWeapon.canFire = true;
	if (!StaticWeaponData->WeaponFunctionalityData.canADSReload)
	{
		CombatState.canAim = true;
	}
	if (bInterrupted)
	{
		return;		//reload canceled, don't grant ammo for a reload that never finished
	}
	
	int32 NewCAM, NewCRA, MagSize;
	if (GetCurrentWeaponStaticData()->InfantryWeaponAmmoData.bCanRoundBeChambered && FMath::IsWithinInclusive(CurrentWeapon.CurrentAmmoinMag, 1, CurrentWeaponStats.MagSize))
	{
		MagSize = CurrentWeaponStats.MagSize + 1;
	}
	else
	{
		MagSize = CurrentWeaponStats.MagSize;
	}
	UBS2FunctionLibrary::CalculateReload(MagSize, CurrentWeapon.CurrentAmmoinMag, CurrentWeapon.CurrentReserveAmmo, NewCAM, NewCRA);
	CurrentWeapon.CurrentAmmoinMag = NewCAM;
	CurrentWeapon.CurrentReserveAmmo = NewCRA;
	if (GetOwnerCharacter()->IsLocallyControlled())
	{
		UBS2FunctionLibrary::GetHUDSubsystem(this)->UpdateStatusHUD_CAMCount(CurrentWeapon.CurrentAmmoinMag);
		UBS2FunctionLibrary::GetHUDSubsystem(this)->UpdateStatusHUD_CRACount(CurrentWeapon.CurrentReserveAmmo);
	}
	UBS2FunctionLibrary::HandleIfWeaponCanFire(CurrentWeapon);
}

#pragma endregion

#pragma region ItemSwitching

void ULoadoutManager::AutoSwitchItem()
{
	//smart switch item instead?
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

void ULoadoutManager::HandleSwitchItem(ELoadoutSlot NewLoadoutSlot)
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
					UnequipBlendOutDelegate.BindUObject(this, &ULoadoutManager::OnUnequipWeapon_BlendOutToWeapon);
					UnequipWeapon(Loadout.PreviousItemIndex);
					break;
				case ECharacterItemType::Gadget:
					UnequipBlendOutDelegate.BindUObject(this, &ULoadoutManager::OnUnequipWeapon_BlendOutToGadget);
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
					UnequipBlendOutDelegate.BindUObject(this, &ULoadoutManager::OnUnequipGadget_BlendOutToWeapon);
					UnequipGadget(Loadout.PreviousItemIndex);
					break;
			}
			break;
	}
}

void ULoadoutManager::TransitionFromItem(int32 PreviousItemIndex, ECharacterItemType LoadoutCategory)
{
	//basically unequip for everything non-anim related
	TWeakObjectPtr<ACharacter_Base> Character = Cast<ACharacter_Base>(GetOwner());
	Character->FPArms->SetVisibility(false);
	switch (LoadoutCategory)
	{
		case ECharacterItemType::Weapon:
			Loadout.WeaponSystem.BaseWeaponState.Weapons[PreviousItemIndex].isEquipped = false;
			UpdateWeaponVisibility(PreviousItemIndex, true);
			break;
		case ECharacterItemType::Gadget:
			UpdateGadgetVisibility(PreviousItemIndex, true);
			break;
	}
	GetWorld()->GetTimerManager().SetTimer(SwitchItemTimer, [this, Character]()
	{
		Character->FPArms->SetVisibility(true);
	}, 0.05f, false);
	
	TransitionToItem();
}

void ULoadoutManager::TransitionToItem()
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

void ULoadoutManager::UnequipWeapon(int32 PreviousWeaponIndex)
{
	TSoftObjectPtr<UAnimMontage> FPUnequipItemMontage = StaticWeaponDataCache[PreviousWeaponIndex]->InfantryWeaponAnimData.FPWeaponAnimData.BaseItemAnimData.UnequipMontage;
	FPUnequipItemMontage.LoadSynchronous();
	ECharacterItemType CurrentItemType = GetCategoryForSlot(Loadout.CurrentSlot);
	TWeakObjectPtr<UAnimInstance> FPArmsAnimInstance = GetOwnerCharacter()->FPArms->GetAnimInstance();
	
	if (Loadout.WeaponSystem.BaseWeaponState.Weapons[PreviousWeaponIndex].isReloading)
	{
		TSoftObjectPtr<UAnimMontage> FPReloadMontage = StaticWeaponDataCache[PreviousWeaponIndex]->InfantryWeaponAnimData.FPWeaponAnimData.ReloadWeaponMontage;
		FPArmsAnimInstance->Montage_Stop(0.1f, FPReloadMontage.Get());
	}
	
	if (!FPUnequipItemMontage.Get())
	{
		TransitionFromItem(PreviousWeaponIndex, ECharacterItemType::Weapon);
		return;
	}
	FPArmsAnimInstance->Montage_Play(FPUnequipItemMontage.Get(), 1.0f, EMontagePlayReturnType::MontageLength, 0.0f);
	FPArmsAnimInstance->Montage_SetBlendingOutDelegate(UnequipBlendOutDelegate, FPUnequipItemMontage.Get());		//BINDING SHOULDVE HAPPENED BEFORE THIS FUNCTION IS CALLED
}

void ULoadoutManager::OnUnequipWeapon_BlendOutToWeapon(UAnimMontage* Montage, bool bInterrupted)
{
	//old slot was weapon, new slot is weapon
	TWeakObjectPtr<ACharacter_Base> Character = GetOwnerCharacter();
	TWeakObjectPtr<UAnimInstance> FPArmsAnimInstance = Character->FPArms->GetAnimInstance();
	TSoftObjectPtr<UAnimMontage> FPUnequipWeaponMontage = StaticWeaponDataCache[Loadout.PreviousItemIndex]->InfantryWeaponAnimData.FPWeaponAnimData.BaseItemAnimData.UnequipMontage;
	if (!FPUnequipWeaponMontage.Get())
	{
		EquipWeapon(GetWeaponIndexForSlot(Loadout.CurrentSlot), false);
		return;
	}
	FPArmsAnimInstance->Montage_Play(FPUnequipWeaponMontage.Get(), 0.0f, EMontagePlayReturnType::MontageLength, FPUnequipWeaponMontage->GetPlayLength());
	TransitionFromItem(Loadout.PreviousItemIndex, ECharacterItemType::Weapon);	//remember, this function calls equip weapon in it
}

void ULoadoutManager::OnUnequipWeapon_BlendOutToGadget(UAnimMontage* Montage, bool bInterrupted)
{
	//old slot was weapon, new slot is gadget
	TWeakObjectPtr<ACharacter_Base> Character = GetOwnerCharacter();
	TWeakObjectPtr<UAnimInstance> FPArmsAnimInstance = Character->FPArms->GetAnimInstance();
	TSoftObjectPtr<UAnimMontage> FPUnequipWeaponMontage = StaticWeaponDataCache[Loadout.PreviousItemIndex]->InfantryWeaponAnimData.FPWeaponAnimData.BaseItemAnimData.UnequipMontage;
	if (!FPUnequipWeaponMontage.Get())
	{
		EquipGadget(GetGadgetIndexForSlot(Loadout.CurrentSlot));	
		return;
	}
	FPArmsAnimInstance->Montage_Play(FPUnequipWeaponMontage.Get(), 0.0f, EMontagePlayReturnType::MontageLength, FPUnequipWeaponMontage->GetPlayLength());
	TransitionFromItem(Loadout.PreviousItemIndex, ECharacterItemType::Weapon);
}

void ULoadoutManager::OnUnequipGadget_BlendOutToWeapon(UAnimMontage* Montage, bool bInterrupted)
{
	TWeakObjectPtr<UAnimInstance> FPArmsAnimInstance = GetOwnerCharacter()->FPArms->GetAnimInstance();
	TSoftObjectPtr<UAnimMontage> FPUnequipGadgetMontage = StaticGadgetDataCache[Loadout.PreviousItemIndex]->GadgetAnimData.BaseItemAnimData.UnequipMontage;
	if (!FPUnequipGadgetMontage.Get())
	{
		EquipWeapon(GetWeaponIndexForSlot(Loadout.CurrentSlot), false);
		return;
	}
	FPArmsAnimInstance->Montage_Play(FPUnequipGadgetMontage.Get(), 0.0f, EMontagePlayReturnType::MontageLength, FPUnequipGadgetMontage->GetPlayLength());
	TransitionFromItem(Loadout.PreviousItemIndex, ECharacterItemType::Gadget);
}

void ULoadoutManager::OnUnequipGadget_BlendOutToGadget(UAnimMontage* Montage, bool bInterrupted)
{
	TWeakObjectPtr<UAnimInstance> FPArmsAnimInstance = GetOwnerCharacter()->FPArms->GetAnimInstance();
	TSoftObjectPtr<UAnimMontage> FPUnequipGadgetMontage = StaticGadgetDataCache[Loadout.PreviousItemIndex]->GadgetAnimData.BaseItemAnimData.UnequipMontage;
	if (!FPUnequipGadgetMontage.Get())
	{
		EquipGadget(GetGadgetIndexForSlot(Loadout.CurrentSlot));	
		return;
	}
	FPArmsAnimInstance->Montage_Play(FPUnequipGadgetMontage.Get(), 0.0f, EMontagePlayReturnType::MontageLength, FPUnequipGadgetMontage->GetPlayLength());
	TransitionFromItem(Loadout.PreviousItemIndex, ECharacterItemType::Gadget);
}

void ULoadoutManager::EquipWeapon(int32 WeaponIndex, bool InitialEquip)
{
	TWeakObjectPtr<ACharacter_Base> Character = GetOwnerCharacter();
	TWeakObjectPtr<UAnimInstance> FPArmsAnimInstance = Character->FPArms->GetAnimInstance();
	const FInfantryWeaponData& StaticWeaponData = *StaticWeaponDataCache[WeaponIndex];
	const FInfantryWeaponAnimData& AnimData = StaticWeaponDataCache[WeaponIndex]->InfantryWeaponAnimData;
	FInfantryWeaponState& InfantryWeaponState_FP = Loadout.WeaponSystem.InfantryWeaponState.WeaponState_FP[WeaponIndex];
	TObjectPtr<USkeletalMeshComponent> NewWeaponMesh = Loadout.WeaponSystem.InfantryWeaponState.WeaponState_FP[WeaponIndex].WeaponMesh.Get();
	UpdateWeaponVisibility(WeaponIndex, false);
	UBS2FunctionLibrary::UpdateWACData(Loadout.WeaponSystem.WeaponAudioComponent, StaticWeaponData.WeaponFirePerformanceData.RateOfFire, StaticWeaponData.InfantryWeaponAudioData.BaseWeaponAudioData);
	TSoftObjectPtr<UAnimMontage> FPEquipWeaponMontage;
	if (InitialEquip)
	{
		//only do weapon mesh animation on initial equip
		TSoftObjectPtr<UAnimSequence> WeaponEquipAnim = AnimData.WeaponAnimData.WeaponEquip;
		WeaponEquipAnim.LoadSynchronous();
		NewWeaponMesh->PlayAnimation(WeaponEquipAnim.Get(), false);
		
		FPEquipWeaponMontage = AnimData.FPWeaponAnimData.BaseItemAnimData.InitialEquipMontage;
	}
	else
	{
		FPEquipWeaponMontage = AnimData.FPWeaponAnimData.BaseItemAnimData.EquipMontage;
	}

	FPEquipWeaponMontage.LoadSynchronous();
	FPArmsAnimInstance->Montage_Play(FPEquipWeaponMontage.Get(), 1.0f);
	IAnims::Execute_OnEquipWeapon_FP(FPArmsAnimInstance.Get(), AnimData.FPWeaponAnimData);
	if (!StaticWeaponDataCache[WeaponIndex]->InfantryWeaponAmmoData.ProjectileBoneToHide.IsNone())
	{
		InfantryWeaponState_FP.WeaponMesh->HideBoneByName(StaticWeaponDataCache[WeaponIndex]->InfantryWeaponAmmoData.ProjectileBoneToHide, EPhysBodyOp::PBO_None);
	}
	UpdateScopeCamera();
	
	FWeaponState& CurrentWeapon = GetBaseWeaponState(GetCII());
	CurrentWeapon.isEquipped = true;
	if (GetOwnerCharacter()->IsLocallyControlled())
	{
		UBS2FunctionLibrary::GetHUDSubsystem(this)->UpdateStatusHUD_CAMCount(CurrentWeapon.CurrentAmmoinMag);
		UBS2FunctionLibrary::GetHUDSubsystem(this)->UpdateStatusHUD_CRACount(CurrentWeapon.CurrentReserveAmmo);
	}
}

#pragma endregion

void ULoadoutManager::ToggleFireMode()
{
	FWeaponStats_Runtime& CurrentWeaponStats = GetCurrentWeaponStats();
	FWeaponFireModeData& CurrentFireModeData = CurrentWeaponStats.FireModeData;
	FWeaponState& CurrentWeapon = *GetCurrentWeaponBaseState();
	EFireMode& CurrentFireMode = CurrentWeapon.CurrentFireMode;

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

void ULoadoutManager::ToggleScope()
{
	if (!CombatState.isAiming) { return; }
	const FWeaponAttachmentData& WeaponAttachmentData = *UBS2FunctionLibrary::GetDataSubsystem(this)->GetWeaponAttachmentDataRow(GetCurrentAttachmentInSlot(EAttachmentSlot::Scope).BaseAttachmentState.AttachmentID);
	int32& CurrentOpticIndex = Loadout.WeaponSystem.InfantryWeaponState.WeaponState_FP[GetCII()].CurrentOpticIndex;
	if (WeaponAttachmentData.WeaponSightData.OpticIDs.Num() > 1)
	{
		int32 TotalOpticLevels = WeaponAttachmentData.WeaponSightData.OpticIDs.Num();
		UBS2FunctionLibrary::UpdateOpticIndex(TotalOpticLevels, CurrentOpticIndex);
		UpdateScope(CurrentOpticIndex);
	}
}

void ULoadoutManager::UpdateScope(int32 NewOpticIndex)
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

void ULoadoutManager::UpdateCurrentWeaponStats(int32 WeaponIndex)
{
	FWeaponStats_Runtime& RuntimeStats = Loadout.WeaponSystem.InfantryWeaponState.CurrentWeaponStats[WeaponIndex];
	FName& WeaponID = Loadout.WeaponSystem.BaseWeaponState.Weapons[WeaponIndex].WeaponID;
	const FInfantryWeaponData* StaticWeaponData = StaticWeaponDataCache[WeaponIndex];
	
	//runtime stats after being modified will be used to fill state
	RuntimeStats.AimInSpeed = StaticWeaponData->InfantryWeaponAimData.DefaultAimInSpeed;
	RuntimeStats.AimOutSpeed = StaticWeaponData->InfantryWeaponAimData.DefaultAimOutSpeed;
	RuntimeStats.SightDistance = StaticWeaponData->InfantryWeaponAimData.DefaultSightDistance;
	RuntimeStats.MagSize = StaticWeaponData->InfantryWeaponAmmoData.BaseAmmoData.MagSize;
	RuntimeStats.MaxReserveAmmo = StaticWeaponData->InfantryWeaponAmmoData.BaseAmmoData.MaxReserveAmmo;
	RuntimeStats.FireModeData.DefaultFireMode = StaticWeaponData->WeaponFunctionalityData.BaseWeaponFunctionality.WeaponFireModeData.DefaultFireMode;
	RuntimeStats.ReloadSpeed = StaticWeaponData->InfantryWeaponAmmoData.BaseAmmoData.ReloadSpeed;

	TMap<EAttachmentSlot, FWeaponAttachmentState>& WeaponAttachmentStates = Loadout.WeaponSystem.InfantryWeaponState.WeaponState_FP[WeaponIndex].WeaponAttachmentStates;
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

void ULoadoutManager::ApplyAttachmentModifier(FWeaponStats_Runtime& RuntimeStats, EWeaponStat WeaponStat, const FStatModifierData& Modifier)
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
		{ EWeaponStat::ReloadSpeed, 
			{ 
			&FWeaponStats_Runtime::ReloadSpeed, 
			nullptr,
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

float ULoadoutManager::CalculateFinalStatValue(float BaseValue, TArray<FStatModifierData>& ModifierArray)
{
	float CurrentValue = BaseValue;
	for (FStatModifierData& Modifier : ModifierArray)
	{
		CurrentValue = Modifier.ApplyToValue(CurrentValue);
	}
	return CurrentValue;
}

#pragma endregion 

#pragma region Gadgets

void ULoadoutManager::UnequipGadget(int32 PreviousGadgetIndex)
{
	TSoftObjectPtr<UAnimMontage> FPUnequipItemMontage = StaticGadgetDataCache[PreviousGadgetIndex]->GadgetAnimData.BaseItemAnimData.UnequipMontage;
	FPUnequipItemMontage.LoadSynchronous();
	ECharacterItemType CurrentItemType = GetCategoryForSlot(Loadout.CurrentSlot);
	if (!FPUnequipItemMontage.Get())
	{
		TransitionFromItem(PreviousGadgetIndex, ECharacterItemType::Gadget);
		return;
	}
	
	TWeakObjectPtr<UAnimInstance> FPArmsAnimInstance = GetOwnerCharacter()->FPArms->GetAnimInstance();
	
	FPArmsAnimInstance->Montage_Play(FPUnequipItemMontage.Get(), 1.0f, EMontagePlayReturnType::MontageLength, 0.0f);
	FPArmsAnimInstance->Montage_SetBlendingOutDelegate(UnequipBlendOutDelegate, FPUnequipItemMontage.Get());		//BINDING SHOULDVE HAPPENED BEFORE THIS FUNCTION IS CALLED
}

void ULoadoutManager::EquipGadget(int32 GadgetIndex)
{
	TWeakObjectPtr<ACharacter_Base> Character = Cast<ACharacter_Base>(GetOwner());
	TWeakObjectPtr<UAnimInstance> FPArmsAnimInstance = Character->FPArms->GetAnimInstance();
	const FGadgetData& GadgetData = *StaticGadgetDataCache[GadgetIndex];
	const FGadgetAnimData& AnimData = GadgetData.GadgetAnimData;
	TObjectPtr<UStaticMeshComponent> NewGadgetMesh = Loadout.Gadgets[GadgetIndex].HeldMesh_FP.Get();
	UpdateGadgetVisibility(GadgetIndex, false);
	NewGadgetMesh->SetRelativeLocation(FVector::ZeroVector);
	
	AnimData.BaseItemAnimData.EquipMontage.LoadSynchronous();
	EquipGadgetBlendOutDelegate.BindUObject(this, &ULoadoutManager::OnEquipGadget_BlendOut);
	FPArmsAnimInstance->Montage_Play(AnimData.BaseItemAnimData.EquipMontage.Get(), 1.0f);
	
	IAnims::Execute_OnEquipGadget(FPArmsAnimInstance.Get(), AnimData);
	
	FPArmsAnimInstance->Montage_SetBlendingOutDelegate(EquipGadgetBlendOutDelegate, AnimData.BaseItemAnimData.EquipMontage.Get());
}

void ULoadoutManager::HandleDeployGadgetInput()
{
	//int32 GadgetIndex = GetCII();
	int32 GadgetIndex = GetGadgetIndexForSlot(Loadout.CurrentSlot);
	if (!Loadout.Gadgets.IsValidIndex(GadgetIndex))
	{
		return; // current slot isn't resolved as a gadget — nothing to deploy
	}
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

void ULoadoutManager::OnEquipGadget_BlendOut(UAnimMontage* Montage, bool bInterrupted)
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

void ULoadoutManager::StartDeployGadget()
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
	UnequipBlendOutDelegate.BindUObject(this, &ULoadoutManager::OnStartDeployGadget_BlendOut);
	FPArmsAnimInstance->Montage_Play(AnimData.DeployGadget.Get(), 1.0f);
	FPArmsAnimInstance->Montage_SetBlendingOutDelegate(UnequipBlendOutDelegate, AnimData.DeployGadget.Get());
}

void ULoadoutManager::OnStartDeployGadget_BlendOut(UAnimMontage* Montage, bool bInterrupted)
{
	DeployGadget();
}

void ULoadoutManager::DeployGadget()
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

void ULoadoutManager::UseGadget()
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
				VehicleGadget->OnVehicleInitialized.AddDynamic(this, &ULoadoutManager::OnDeployedVehicleGadgetReady);
			}
			break;
	}
	
}

void ULoadoutManager::OnDeployedVehicleGadgetReady()
{
	int32 GadgetIndex = GetCII();
	FGadgetState& GadgetState = Loadout.Gadgets[GadgetIndex];
	TObjectPtr<AVehicle_Base> VehicleGadget = Cast<AVehicle_Base>(GadgetState.ActivePlacedInstances[0]);
	check (VehicleGadget);
	VehicleGadget->OnVehicleInitialized.RemoveDynamic(this, &ULoadoutManager::OnDeployedVehicleGadgetReady);
	VehicleGadget->AttemptEnterVehicle(GetOwnerCharacter());
}

#pragma endregion

#pragma region Getters

ECharacterItemType ULoadoutManager::GetCategoryForSlot(ELoadoutSlot LoadoutSlot)
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

int32 ULoadoutManager::GetArrayIndex(ELoadoutSlot LoadoutSlot)
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

int32 ULoadoutManager::GetWeaponIndexForSlot(ELoadoutSlot LoadoutSlot)
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

int32 ULoadoutManager::GetGadgetIndexForSlot(ELoadoutSlot LoadoutSlot)
{
	int32 SlotPosition = GetArrayIndex(LoadoutSlot);
	const FResolvedGadgetSlot& ResolvedGadgetSlot = Loadout.ResolvedGadgetSlots[SlotPosition];
	return (ResolvedGadgetSlot.ActualType == ECharacterItemType::Gadget) ? ResolvedGadgetSlot.ResolvedArrayIndex : INDEX_NONE;
}

ECharacterItemType ULoadoutManager::GetActualGadgetItemType(int32 RawGadgetSlotIndex)
{
	return Loadout.ResolvedGadgetSlots.IsValidIndex(RawGadgetSlotIndex) ? Loadout.ResolvedGadgetSlots[RawGadgetSlotIndex].ActualType : ECharacterItemType::Gadget;
}

bool ULoadoutManager::GetIsCurrentSlotActuallyWeapon()
{
	ECharacterItemType SlotCategory = GetCategoryForSlot(Loadout.CurrentSlot);
	if (SlotCategory == ECharacterItemType::Weapon)
	{
		return true;
	}
	int32 RawSlotPosition = GetArrayIndex(Loadout.CurrentSlot);
	ECharacterItemType ActualType = GetActualGadgetItemType(RawSlotPosition);
	
	return ActualType == ECharacterItemType::Weapon;
}

int32 ULoadoutManager::GetMaxMagSize(int32 WeaponIndex)
{
	const bool& bCanBeChambered = StaticWeaponDataCache[WeaponIndex]->InfantryWeaponAmmoData.bCanRoundBeChambered;
	const int32 MagSize = Loadout.WeaponSystem.InfantryWeaponState.CurrentWeaponStats[WeaponIndex].MagSize;
	return bCanBeChambered ? MagSize + 1 : MagSize;
}

FName ULoadoutManager::GetSocketNameForSlot(EAttachmentSlot Slot)
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

FTransform ULoadoutManager::GetSightTransform()
{
	FTransform SightTransform = Loadout.WeaponSystem.InfantryWeaponState.WeaponState_FP[GetCII()].WeaponMesh->GetSocketTransform(FName("Aimpoint"), ERelativeTransformSpace::RTS_Component);
	FInfantryWeaponState& InfantryWeaponState = Loadout.WeaponSystem.InfantryWeaponState.WeaponState_FP[GetCII()];
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

float ULoadoutManager::GetSightDistance()
{
	FName& WeaponID = Loadout.WeaponSystem.BaseWeaponState.Weapons[GetCII()].WeaponID;
	const float& DefaultSightDistance = UBS2FunctionLibrary::GetDataSubsystem(this)->GetInfantryWeaponDataRow(WeaponID)->InfantryWeaponAimData.DefaultSightDistance;
	FInfantryWeaponState& InfantryWeaponState = Loadout.WeaponSystem.InfantryWeaponState.WeaponState_FP[GetCII()];
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

void ULoadoutManager::GetAimSpeeds(float& AimInSpeed, float& AimOutSpeed)
{
	FName& WeaponID = Loadout.WeaponSystem.BaseWeaponState.Weapons[GetCII()].WeaponID;
	const float& DefaultAimInSpeed = UBS2FunctionLibrary::GetDataSubsystem(this)->GetInfantryWeaponDataRow(WeaponID)->InfantryWeaponAimData.DefaultAimInSpeed;
	const float& DefaultAimOutSpeed = UBS2FunctionLibrary::GetDataSubsystem(this)->GetInfantryWeaponDataRow(WeaponID)->InfantryWeaponAimData.DefaultAimOutSpeed;
	TMap<EAttachmentSlot, FWeaponAttachmentState>& WeaponAttachmentStates = Loadout.WeaponSystem.InfantryWeaponState.WeaponState_FP[GetCII()].WeaponAttachmentStates;
	TArray<FStatModifierData> ADSModifiers;
	GetAllAttachmentModifierDataOfTypeForWeapon(WeaponAttachmentStates, EWeaponStat::ADSInSpeed, ADSModifiers);
	float CurrentAimInSpeed = CalculateFinalStatValue(DefaultAimInSpeed, ADSModifiers);
	AimInSpeed = CurrentAimInSpeed;
	AimOutSpeed = DefaultAimOutSpeed;
}

void ULoadoutManager::GetAllAttachmentModifierDataOfTypeForWeapon(TMap<EAttachmentSlot, FWeaponAttachmentState>& WeaponAttachmentStates, EWeaponStat WeaponStatType, TArray<FStatModifierData>& OutAttachmentModifierData)
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

FWeaponAttachmentState& ULoadoutManager::GetCurrentAttachmentInSlot(EAttachmentSlot Slot)
{
	if (Loadout.WeaponSystem.InfantryWeaponState.WeaponState_FP[GetCII()].WeaponAttachmentStates.Contains(Slot))
	{
		return Loadout.WeaponSystem.InfantryWeaponState.WeaponState_FP[GetCII()].WeaponAttachmentStates[Slot];
	}
	else
	{
		static FWeaponAttachmentState DefaultAttachmentState;
		return DefaultAttachmentState;
	}
}

FWeaponState& ULoadoutManager::GetBaseWeaponState(int32 WeaponIndex)
{
	return Loadout.WeaponSystem.BaseWeaponState.Weapons[WeaponIndex];
}

FInfantryWeaponState& ULoadoutManager::GetCurrentInfantryWeaponState_FP()
{
	return Loadout.WeaponSystem.InfantryWeaponState.WeaponState_FP[GetCII()];
}

FVector ULoadoutManager::GetAttachmentDefaultOffset(FName WeaponID, EAttachmentSlot Slot, FName AttachmentID)
{
	UE_LOG(LogTemp, Warning, TEXT("[WeaponLogicComponent::GetAttachmentDefaultOffset] WeaponID %s, AttachmentID %s"), *WeaponID.ToString(), *AttachmentID.ToString());
	return UBS2FunctionLibrary::GetDataSubsystem(this)->GetInfantryWeaponDataRow(WeaponID)->GunAttachmentData.AvailableAttachmentSlots.Find(Slot)->Attachments.Find(AttachmentID)->LocationOffset;
}

FWeaponState* ULoadoutManager::GetCurrentWeaponBaseState()
{
	//ASSUMES CURRENT SLOT IS WEAPON
	FWeaponState* CurrentWeapon = &Loadout.WeaponSystem.BaseWeaponState.Weapons[GetCII()];
	return CurrentWeapon;
}

const FInfantryWeaponData* ULoadoutManager::GetCurrentWeaponStaticData() 
{
	//assumes currentitemindex is correct and that current slot is weapon
	const int32 WeaponIndex = GetCII();
	const FInfantryWeaponData* StaticWeaponData = StaticWeaponDataCache[WeaponIndex];
	return StaticWeaponData;
}

FInfantryWeaponData ULoadoutManager::GetCurrentWeaponStaticData_BP()
{
	const FInfantryWeaponData* StaticWeaponData = GetCurrentWeaponStaticData();
	return *StaticWeaponData;
}

FWeaponStats_Runtime& ULoadoutManager::GetCurrentWeaponStats()
{
	return Loadout.WeaponSystem.InfantryWeaponState.CurrentWeaponStats[GetCII()];
}

int32 ULoadoutManager::GetCII()
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

ACharacter_Base* ULoadoutManager::GetOwnerCharacter()
{
	return Cast<ACharacter_Base>(GetOwner());
}

#pragma endregion

