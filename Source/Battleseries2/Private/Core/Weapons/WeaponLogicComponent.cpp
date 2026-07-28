
#include "Core/Weapons/WeaponLogicComponent.h"
#include "Core/Weapons/WeaponFunctions.h"
#include "Character_Base.h"
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

void UWeaponLogicComponent::Init_WeaponLoadout(FPlayerLoadoutConfig_Class ClassLoadout, TArray<FPlayerLoadoutConfig_Weapon> WeaponLoadouts)
{
	TArray<FName>& Weapons = ClassLoadout.Weapons;
	WeaponSystem.BaseWeaponSystem.Weapons.SetNum(Weapons.Num());
	StaticWeaponDataCache.SetNum(Weapons.Num());
	WeaponSystem.InfantryWeaponSystem.WeaponState_FP.SetNum(Weapons.Num());
	WeaponSystem.InfantryWeaponSystem.WeaponState_TP.SetNum(Weapons.Num());
	WeaponSystem.InfantryWeaponSystem.CurrentWeaponStats.SetNum(Weapons.Num());

	for (int32 i = 0; i < Weapons.Num(); i++)
	{
		Init_Weapon(Weapons[i], i, WeaponLoadouts[i]);
	}
	Init_ScopeCamera();
	EquipWeapon(0, true);
}

void UWeaponLogicComponent::Init_ScopeCamera()
{
	WeaponSystem.ScopeCamera = NewObject<USceneCaptureComponent2D>(GetOwner());
	WeaponSystem.ScopeCamera->RegisterComponent();
	WeaponSystem.ScopeCamera->TextureTarget = UKismetRenderingLibrary::CreateRenderTarget2D(this, 256, 256, RTF_RGBA16f);
	WeaponSystem.ScopeCamera->HideActorComponents(GetOwner(), true);
	WeaponSystem.ScopeCamera->bCaptureEveryFrame = true;
	WeaponSystem.ScopeCamera->CaptureSource = ESceneCaptureSource::SCS_SceneColorHDR;
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
	WeaponSystem.InfantryWeaponSystem.WeaponState_FP[WeaponIndex] = NewFPState;
	
	FString SocketString = FString::Printf(TEXT("Socket_%s"), *WeaponID.ToString());
	FName AttachSocketName = FName(*SocketString);

	WeaponSystem.InfantryWeaponSystem.WeaponState_FP[WeaponIndex].WeaponMesh->AttachToComponent(Cast<ACharacter_Base>(GetOwner())->FPArms, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, true), AttachSocketName);
	
	WeaponSystem.BaseWeaponSystem.Weapons[WeaponIndex].WeaponID = WeaponID;
	StaticWeaponDataCache[WeaponIndex] = UBS2FunctionLibrary::GetDataSubsystem(this)->GetInfantryWeaponDataRow(WeaponID);

	//setup custom weapon (attachments, stats, etc)
	//apply saved attachments
	const FPlayerLoadoutConfig_Weapon& CustomWeapon = WeaponLoadout;
	ApplyAttachments(CustomWeapon, WeaponIndex);

	UpdateCurrentWeaponStats(WeaponIndex);

	//weaponstate
	FWeaponStats_Runtime& CurrentWeaponStats = WeaponSystem.InfantryWeaponSystem.CurrentWeaponStats[WeaponIndex];
	FWeapon_Runtime& BaseWeaponState = GetBaseWeaponState(WeaponIndex);
	BaseWeaponState.WeaponState.CurrentAmmoinMag = CurrentWeaponStats.MagSize;
	BaseWeaponState.WeaponState.CurrentReserveAmmo = CurrentWeaponStats.MaxReserveAmmo;
	BaseWeaponState.WeaponState.CurrentFireMode = CurrentWeaponStats.FireModeData.DefaultFireMode;
	BaseWeaponState.WeaponID = WeaponID;
	
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

void UWeaponLogicComponent::ApplyAttachments(const FPlayerLoadoutConfig_Weapon& AttachmentsToApply, int32 WeaponIndex)
{
	FInfantryWeaponState& WeaponToApplyTo = WeaponSystem.InfantryWeaponSystem.WeaponState_FP[WeaponIndex];
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

void UWeaponLogicComponent::UpdateWeaponData(int32 WeaponIndex, FName WeaponID, FInfantryWeaponState WeaponState)
{
	WeaponSystem.BaseWeaponSystem.Weapons[WeaponIndex].WeaponID = WeaponID;
	StaticWeaponDataCache[WeaponIndex] = UBS2FunctionLibrary::GetDataSubsystem(this)->GetInfantryWeaponDataRow(WeaponID);

	//add a bool or an int if it should fp, tp or both
	WeaponSystem.InfantryWeaponSystem.WeaponState_FP[WeaponIndex] = WeaponState;
}

void UWeaponLogicComponent::UpdateScopeCamera()
{
	//called on equip weapon
	WeaponSystem.ScopeCamera->AttachToComponent(WeaponSystem.InfantryWeaponSystem.WeaponState_FP[GetCWI()].WeaponMesh.Get(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, true), FName("PIP"));

	if (WeaponSystem.InfantryWeaponSystem.WeaponState_FP[GetCWI()].WeaponAttachmentStates.Find(EAttachmentSlot::Scope))
	{

		FWeaponAttachmentState& WeaponAttachmentState = *WeaponSystem.InfantryWeaponSystem.WeaponState_FP[GetCWI()].WeaponAttachmentStates.Find(EAttachmentSlot::Scope);
		int32 PIPMatIndex = UBS2FunctionLibrary::GetDataSubsystem(this)->GetWeaponAttachmentDataRow(WeaponAttachmentState.BaseAttachmentState.AttachmentID)->WeaponSightData.PIPMaterialIndex;
		if (PIPMatIndex < 0) { return; }
		UMaterialInstanceDynamic* MID = WeaponAttachmentState.SpawnedAttachment->CreateDynamicMaterialInstance(PIPMatIndex, WeaponAttachmentState.SpawnedAttachment->GetMaterial(PIPMatIndex));
		WeaponAttachmentState.SpawnedAttachment->SetMaterial(PIPMatIndex, MID);
		MID->SetTextureParameterValue(FName("RenderTarget"), WeaponSystem.ScopeCamera->TextureTarget);
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
	WeaponSystem.InfantryWeaponSystem.WeaponState_FP[WeaponIndex].WeaponMesh->SetCollisionResponseToChannel(CollisionChannel, CollisionResponse);
	for (auto& Attachment : WeaponSystem.InfantryWeaponSystem.WeaponState_FP[WeaponIndex].WeaponAttachmentStates)
	{
		Attachment.Value.SpawnedAttachment.Get()->SetCollisionResponseToChannel(CollisionChannel, CollisionResponse);
	}
	//WeaponSystem.InfantryWeaponSystem.WeaponState_TP[WeaponSystem.BaseWeaponSystem.EquippedWeaponState.CurrentWeaponIndex].WeaponMesh->SetCollisionResponseToChannel(CollisionChannel, CollisionResponse);
}

void UWeaponLogicComponent::UpdateWeaponVisibility(int32 WeaponIndex, bool Hide)
{
	FInfantryWeaponState& Weapon = WeaponSystem.InfantryWeaponSystem.WeaponState_FP[WeaponIndex];
	Weapon.WeaponMesh->SetHiddenInGame(Hide);
	for (auto& AttachmentSlot : Weapon.WeaponAttachmentStates)
	{
		AttachmentSlot.Value.SpawnedAttachment->SetHiddenInGame(Hide);
	}
}

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
	WeaponSystem.isAiming = true;
}

void UWeaponLogicComponent::StopAim()
{
	const FInfantryWeaponAimData& AimData = GetCurrentWeaponStaticData()->InfantryWeaponAimData;
	if (!AimData.canAim) { return; }
	if (!WeaponSystem.isAiming) { return; }
	if (AimData.HideArms)
	{
		TWeakObjectPtr<ACharacter_Base> Character = Cast<ACharacter_Base>(GetOwner());
		USkeletalMeshComponent* FPArms = Character->FPArms;
		FPArms->SetVisibility(true);
		FString SocketString = FString::Printf(TEXT("Socket_%s"), *GetCurrentWeaponRuntime()->WeaponID.ToString());
		FName AttachSocketName = FName(*SocketString);
		GetCurrentInfantryWeaponState_FP().WeaponMesh->AttachToComponent(Character->FPArms, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, true), AttachSocketName);
	}
	WeaponSystem.isAiming = false;
}

void UWeaponLogicComponent::Rangefinder()
{
	FHitResult OutHit;
	UBS2FunctionLibrary::PerformWeaponLineTrace(this, Cast<ACharacter_Base>(GetOwner())->FPCamera->GetComponentTransform(), OutHit, { GetOwner() }, false);
	TWeakObjectPtr<USkeletalMeshComponent>& WeaponMesh = WeaponSystem.InfantryWeaponSystem.WeaponState_FP[GetCWI()].WeaponMesh;

	WeaponSystem.BaseWeaponSystem.EquippedWeaponState.RaycastData.RangefinderData = OutHit;
	WeaponSystem.BaseWeaponSystem.EquippedWeaponState.RaycastData.MuzzleAimDirections[0] = UBS2FunctionLibrary::GetAimDirectionFromMuzzle(OutHit, FName("Muzzle"), WeaponMesh);
}

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
		FVector& AimDirection = WeaponSystem.BaseWeaponSystem.EquippedWeaponState.RaycastData.MuzzleAimDirections[0];
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

void UWeaponLogicComponent::ReloadWeapon()
{
	FWeapon_Runtime& CurrentWeapon = *GetCurrentWeaponRuntime();
	const FInfantryWeaponData* StaticWeaponData = GetCurrentWeaponStaticData();
	FInfantryWeaponState& InfantryWeaponState_FP = WeaponSystem.InfantryWeaponSystem.WeaponState_FP[GetCWI()];
	ReloadEndedDelegate.BindUObject(this, &UWeaponLogicComponent::OnReloadFinished);

	if (CurrentWeapon.WeaponState.CurrentReserveAmmo > 0)
	{ 
		CurrentWeapon.WeaponState.isReloading = true;
		bool bEmptyMag = CurrentWeapon.WeaponState.CurrentAmmoinMag <= 0;

		TWeakObjectPtr<ACharacter_Base> Character = Cast<ACharacter_Base>(GetOwner());
		USkeletalMeshComponent* FPArms = Character->FPArms;
		FString SocketString = FString::Printf(TEXT("Socket_%s_R"), *GetCurrentWeaponRuntime()->WeaponID.ToString());
		FName AttachSocketName = FName(*SocketString);
		if (FPArms->DoesSocketExist(AttachSocketName))
		{
			InfantryWeaponState_FP.WeaponMesh->AttachToComponent(Character->FPArms, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, true), AttachSocketName);
		}

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
	FInfantryWeaponState& InfantryWeaponState_FP = WeaponSystem.InfantryWeaponSystem.WeaponState_FP[GetCWI()];

	TWeakObjectPtr<ACharacter_Base> Character = Cast<ACharacter_Base>(GetOwner());
	USkeletalMeshComponent* FPArms = Character->FPArms;
	FString ReloadSocketString = FString::Printf(TEXT("Socket_%s_R"), *GetCurrentWeaponRuntime()->WeaponID.ToString());
	FString SocketString = FString::Printf(TEXT("Socket_%s"), *GetCurrentWeaponRuntime()->WeaponID.ToString());
	FName ReloadAttachSocketName = FName(*ReloadSocketString);
	FName AttachSocketName = FName(*SocketString);
	if (FPArms->DoesSocketExist(ReloadAttachSocketName))			//assumes attached to ReloadSocket
	{
		InfantryWeaponState_FP.WeaponMesh->AttachToComponent(Character->FPArms, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, true), AttachSocketName);
	}


	int32 NewCAM, NewCRA;
	UBS2FunctionLibrary::CalculateReload(CurrentWeaponStats.MagSize, CurrentWeapon.WeaponState.CurrentAmmoinMag, CurrentWeapon.WeaponState.CurrentReserveAmmo, NewCAM, NewCRA);
	CurrentWeapon.WeaponState.CurrentAmmoinMag = NewCAM;
	CurrentWeapon.WeaponState.CurrentReserveAmmo = NewCRA;
	CurrentWeapon.WeaponState.isReloading = false;
}

void UWeaponLogicComponent::SwitchWeapon_AutoIncrement()
{
	//use to auto switch/revolve/rotate between weapons in a list
	int32 NewWeaponIndex = 0;
	UBS2FunctionLibrary::UpdateWeaponIndex(WeaponSystem.BaseWeaponSystem.Weapons, GetCWI(), NewWeaponIndex);
	StartSwitchWeapon(GetCWI(), NewWeaponIndex);
}

void UWeaponLogicComponent::StartSwitchWeapon(int32 LastWeaponIndex, int32 NewWeaponIndex)
{
	//main switch weapon function, can be used to directly equip a given index (manual) or auto increment (via SwitchWeapon_AutoIncrement)
	WeaponSystem.PreviousWeaponIndex = LastWeaponIndex;
	GetCWI() = NewWeaponIndex;
	TSoftObjectPtr<UAnimMontage> FPUnequipWeaponMontage = StaticWeaponDataCache[LastWeaponIndex]->InfantryWeaponAnimData.FPWeaponAnimData.UnequipWeaponMontage;
	FPUnequipWeaponMontage.LoadSynchronous();
	if (!FPUnequipWeaponMontage.Get())
	{
		TransitionWeapon();
		return;
	}
	TWeakObjectPtr<UAnimInstance> FPArmsAnimInstance = Cast<ACharacter_Base>(GetOwner())->FPArms->GetAnimInstance();
	UnequipBlendOutDelegate.BindUObject(this, &UWeaponLogicComponent::OnUnequipBlendOut);
	FPArmsAnimInstance->Montage_Play(FPUnequipWeaponMontage.Get(), 1.0f, EMontagePlayReturnType::MontageLength, 0.0f);
	FPArmsAnimInstance->Montage_SetBlendingOutDelegate(UnequipBlendOutDelegate, FPUnequipWeaponMontage.Get());
}

void UWeaponLogicComponent::TransitionWeapon()
{
	TWeakObjectPtr<ACharacter_Base> Character = Cast<ACharacter_Base>(GetOwner());
	Character->FPArms->SetVisibility(false);
	UpdateWeaponVisibility(WeaponSystem.PreviousWeaponIndex, true);
	GetWorld()->GetTimerManager().SetTimer(SwitchWeaponTimer, [this, Character]()
	{
		Character->FPArms->SetVisibility(true);
	}, 0.05f, false);

	EquipWeapon(GetCWI(), false);
}

void UWeaponLogicComponent::OnUnequipBlendOut(UAnimMontage* Montage, bool bInterrupted)
{
	TWeakObjectPtr<ACharacter_Base> Character = Cast<ACharacter_Base>(GetOwner());
	TWeakObjectPtr<UAnimInstance> FPArmsAnimInstance = Character->FPArms->GetAnimInstance();
	TSoftObjectPtr<UAnimMontage> FPUnequipWeaponMontage = StaticWeaponDataCache[WeaponSystem.PreviousWeaponIndex]->InfantryWeaponAnimData.FPWeaponAnimData.UnequipWeaponMontage;
	if (!FPUnequipWeaponMontage.Get())
	{
		EquipWeapon(GetCWI(), false);
		return;
	}
	FPArmsAnimInstance->Montage_Play(FPUnequipWeaponMontage.Get(), 0.0f, EMontagePlayReturnType::MontageLength, FPUnequipWeaponMontage->GetPlayLength());

	TransitionWeapon();
}

void UWeaponLogicComponent::EquipWeapon(int32 WeaponIndex, bool InitialEquip)
{
	TWeakObjectPtr<ACharacter_Base> Character = Cast<ACharacter_Base>(GetOwner());
	TWeakObjectPtr<UAnimInstance> FPArmsAnimInstance = Character->FPArms->GetAnimInstance();
	TObjectPtr<USkeletalMeshComponent> NewWeaponMesh = WeaponSystem.InfantryWeaponSystem.WeaponState_FP[WeaponIndex].WeaponMesh.Get();
	UpdateWeaponVisibility(WeaponIndex, false);
	TSoftObjectPtr<UAnimMontage> FPEquipWeaponMontage;
	if (InitialEquip)
	{
		//only do weapon mesh animation on initial equip
		TSoftObjectPtr<UAnimSequence> WeaponEquipAnim = GetCurrentWeaponStaticData()->InfantryWeaponAnimData.WeaponAnimData.WeaponEquip;
		WeaponEquipAnim.LoadSynchronous();
		NewWeaponMesh->PlayAnimation(WeaponEquipAnim.Get(), false);
		FPEquipWeaponMontage = GetCurrentWeaponStaticData()->InfantryWeaponAnimData.FPWeaponAnimData.InitialEquipWeaponMontage;
	}
	else
	{
		FPEquipWeaponMontage = GetCurrentWeaponStaticData()->InfantryWeaponAnimData.FPWeaponAnimData.EquipWeaponMontage;
	}

	FPEquipWeaponMontage.LoadSynchronous();
	FPArmsAnimInstance->Montage_Play(FPEquipWeaponMontage.Get(), 1.0f);
	IAnims::Execute_OnEquipWeapon_FP(FPArmsAnimInstance.Get(), GetCurrentWeaponStaticData()->InfantryWeaponAnimData.FPWeaponAnimData);
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

void UWeaponLogicComponent::ToggleScope()
{
	if (!WeaponSystem.isAiming) { return; }
	const FWeaponAttachmentData& WeaponAttachmentData = *UBS2FunctionLibrary::GetDataSubsystem(this)->GetWeaponAttachmentDataRow(GetCurrentAttachmentInSlot(EAttachmentSlot::Scope).BaseAttachmentState.AttachmentID);
	int32& CurrentOpticIndex = WeaponSystem.InfantryWeaponSystem.WeaponState_FP[GetCWI()].CurrentOpticIndex;
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
		if (WeaponSystem.ScopeCamera->CaptureSource != ESceneCaptureSource::SCS_FinalColorLDR)
		{
			WeaponSystem.ScopeCamera->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
		}
	}
	else if (WeaponSystem.ScopeCamera->CaptureSource != ESceneCaptureSource::SCS_SceneColorHDR)
	{
		WeaponSystem.ScopeCamera->CaptureSource = ESceneCaptureSource::SCS_SceneColorHDR;
	}

	UBS2FunctionLibrary::HandleUpdateOptic(DefaultFOV, OpticData.ZoomMagnification, WeaponSystem.ScopeCamera->FOVAngle, OpticData.OpticPPSettings, WeaponSystem.ScopeCamera->PostProcessSettings, WeaponSystem.ScopeCamera->PostProcessBlendWeight);
}

void UWeaponLogicComponent::UpdateCurrentWeaponStats(int32 WeaponIndex)
{
	FWeaponStats_Runtime& RuntimeStats = WeaponSystem.InfantryWeaponSystem.CurrentWeaponStats[WeaponIndex];
	FName& WeaponID = WeaponSystem.BaseWeaponSystem.Weapons[WeaponIndex].WeaponID;
	const FInfantryWeaponData* StaticWeaponData = StaticWeaponDataCache[WeaponIndex];

	RuntimeStats.AimInSpeed = StaticWeaponData->InfantryWeaponAimData.DefaultAimInSpeed;
	RuntimeStats.AimOutSpeed = StaticWeaponData->InfantryWeaponAimData.DefaultAimOutSpeed;
	RuntimeStats.SightDistance = StaticWeaponData->InfantryWeaponAimData.DefaultSightDistance;
	RuntimeStats.MagSize = StaticWeaponData->InfantryWeaponAmmoData.BaseAmmoData.MagSize;
	RuntimeStats.MaxReserveAmmo = StaticWeaponData->InfantryWeaponAmmoData.BaseAmmoData.MaxReserveAmmo;

	TMap<EAttachmentSlot, FWeaponAttachmentState>& WeaponAttachmentStates = WeaponSystem.InfantryWeaponSystem.WeaponState_FP[WeaponIndex].WeaponAttachmentStates;
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

#pragma region Getters

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
	FTransform SightTransform = WeaponSystem.InfantryWeaponSystem.WeaponState_FP[GetCWI()].WeaponMesh->GetSocketTransform(FName("Aimpoint"), ERelativeTransformSpace::RTS_Component);
	FInfantryWeaponState& InfantryWeaponState = WeaponSystem.InfantryWeaponSystem.WeaponState_FP[GetCWI()];
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
	FName& WeaponID = WeaponSystem.BaseWeaponSystem.Weapons[GetCWI()].WeaponID;
	const float& DefaultSightDistance = UBS2FunctionLibrary::GetDataSubsystem(this)->GetInfantryWeaponDataRow(WeaponID)->InfantryWeaponAimData.DefaultSightDistance;
	FInfantryWeaponState& InfantryWeaponState = WeaponSystem.InfantryWeaponSystem.WeaponState_FP[GetCWI()];
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
	FName& WeaponID = WeaponSystem.BaseWeaponSystem.Weapons[GetCWI()].WeaponID;
	const float& DefaultAimInSpeed = UBS2FunctionLibrary::GetDataSubsystem(this)->GetInfantryWeaponDataRow(WeaponID)->InfantryWeaponAimData.DefaultAimInSpeed;
	const float& DefaultAimOutSpeed = UBS2FunctionLibrary::GetDataSubsystem(this)->GetInfantryWeaponDataRow(WeaponID)->InfantryWeaponAimData.DefaultAimOutSpeed;
	TMap<EAttachmentSlot, FWeaponAttachmentState>& WeaponAttachmentStates = WeaponSystem.InfantryWeaponSystem.WeaponState_FP[GetCWI()].WeaponAttachmentStates;
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
	if (WeaponSystem.InfantryWeaponSystem.WeaponState_FP[GetCWI()].WeaponAttachmentStates.Contains(Slot))
	{
		return WeaponSystem.InfantryWeaponSystem.WeaponState_FP[GetCWI()].WeaponAttachmentStates[Slot];
	}
	else
	{
		static FWeaponAttachmentState DefaultAttachmentState;
		return DefaultAttachmentState;
	}
}

FWeapon_Runtime& UWeaponLogicComponent::GetBaseWeaponState(int32 WeaponIndex)
{
	return WeaponSystem.BaseWeaponSystem.Weapons[WeaponIndex];
}

FInfantryWeaponState& UWeaponLogicComponent::GetCurrentInfantryWeaponState_FP()
{
	return WeaponSystem.InfantryWeaponSystem.WeaponState_FP[GetCWI()];
}

FVector UWeaponLogicComponent::GetAttachmentDefaultOffset(FName WeaponID, EAttachmentSlot Slot, FName AttachmentID)
{
	return UBS2FunctionLibrary::GetDataSubsystem(this)->GetInfantryWeaponDataRow(WeaponID)->GunAttachmentData.AvailableAttachmentSlots.Find(Slot)->Attachments.Find(AttachmentID)->LocationOffset;
}


FWeapon_Runtime* UWeaponLogicComponent::GetCurrentWeaponRuntime()
{
	FWeapon_Runtime* CurrentWeapon = &WeaponSystem.BaseWeaponSystem.Weapons[GetCWI()];
	return CurrentWeapon;
}

const FInfantryWeaponData* UWeaponLogicComponent::GetCurrentWeaponStaticData() const
{
	const FInfantryWeaponData* StaticWeaponData = StaticWeaponDataCache[WeaponSystem.BaseWeaponSystem.EquippedWeaponState.CurrentWeaponIndex];
	return StaticWeaponData;
}

FInfantryWeaponData UWeaponLogicComponent::GetCurrentWeaponStaticData_BP()
{
	const FInfantryWeaponData* StaticWeaponData = GetCurrentWeaponStaticData();
	return *StaticWeaponData;
}

FWeaponStats_Runtime& UWeaponLogicComponent::GetCurrentWeaponStats()
{
	return WeaponSystem.InfantryWeaponSystem.CurrentWeaponStats[GetCWI()];
}

int32& UWeaponLogicComponent::GetCWI()
{
	return WeaponSystem.BaseWeaponSystem.EquippedWeaponState.CurrentWeaponIndex;
}

#pragma endregion

