
#include "Core/Weapons/WeaponLogicComponent.h"
#include "Core/Weapons/WeaponFunctions.h"
#include "Character_Base.h"
#include "Data/Core/CoreTypes.h"
#include "Data/Runtime/ProjectileTypes.h"
#include "Data/Weapons/Data_Weapon.h"
#include "Data/Weapons/Data_InfantryWeapon.h"
#include "Data/Weapons/Data_WeaponAttachments.h"
#include "Data/Weapons/Data_Projectile.h"
#include "Save/SaveSubsystem.h"

UWeaponLogicComponent::UWeaponLogicComponent()
{

}

void UWeaponLogicComponent::BeginPlay()
{
	Super::BeginPlay();

	ProjectileManager = GetWorld()->GetSubsystem<UProjectilePoolSubsystem>();
}

void UWeaponLogicComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UWeaponLogicComponent::Init_WeaponLoadout(FPlayerLoadoutConfig_Class Loadout)
{
	TArray<FName>& Weapons = Loadout.Weapons;
	for (int32 i = 0; i < Weapons.Num(); i++)
	{
		FInfantryWeaponState NewFPState;
		//Create Weapon Mesh
		TWeakObjectPtr<USkeletalMeshComponent> NewWeapon = NewObject<USkeletalMeshComponent>(GetOwner());
		NewWeapon->RegisterComponent();
		//Update Mesh
		const FInfantryWeaponData& WeaponData = *GetDataManager()->GetInfantryWeaponDataRow(Weapons[i]);
		TWeakObjectPtr<USkeletalMesh> WeaponMesh = WeaponData.WeaponClassificationData.WeaponMesh.LoadSynchronous();
		NewWeapon->SetSkeletalMesh(WeaponMesh.Get());
		NewFPState.WeaponMesh = NewWeapon;
		//apply saved attachments
		USaveSubsystem* SaveSys = GetWorld()->GetGameInstance()->GetSubsystem<USaveSubsystem>();
		const FPlayerLoadoutConfig_Weapon& CustomWeapon = SaveSys->GetWeaponLoadout(Weapons[i]);
		ApplyAttachments(CustomWeapon, NewFPState);
		//NewWeapon->SetHiddenInGame(true);
		WeaponSystem.InfantryWeaponSystem.WeaponState_FP.Add(NewFPState);
		NewWeapon->AttachToComponent(Cast<ACharacter_Base>(GetOwner())->GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepRelative, true));
	}

}

void UWeaponLogicComponent::ApplyAttachments(const FPlayerLoadoutConfig_Weapon& AttachmentsToApply, FInfantryWeaponState WeaponToApplyTo)
{
	for (auto& Slot : AttachmentsToApply.WeaponAttachments)
	{
		const EAttachmentSlot& SlotType = Slot.Key;
		const FPlayerLoadoutConfig_WeaponAttachment& AttachmentConfig = Slot.Value;
		//create Attachment
		TWeakObjectPtr<UStaticMeshComponent> NewAttachment = NewObject<UStaticMeshComponent>(GetOwner());
		NewAttachment->RegisterComponent();
		//update attachment mesh
		const FWeaponAttachmentData& AttachmentData = *GetDataManager()->GetWeaponAttachmentDataRow(TEXT("A_Holo_1"));
		TWeakObjectPtr<UStaticMesh> AttachmentMesh = AttachmentData.AttachmentClassification.AttachmentMesh.LoadSynchronous();
		NewAttachment->SetStaticMesh(AttachmentMesh.Get());
		//attach to gun
		NewAttachment->AttachToComponent
		(
			WeaponToApplyTo.WeaponMesh.Get(),
			FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepRelative, true), 
			GetSocketNameForSlot(SlotType)
		);
		//cache runtime state data
		FWeaponAttachmentState& RuntimeSlotState = WeaponToApplyTo.WeaponAttachmentStates.FindOrAdd(SlotType);
		RuntimeSlotState.BaseAttachmentState.AttachmentID = AttachmentConfig.AttachmentID;
		RuntimeSlotState.SpawnedAttachment = NewAttachment;
	}
}

bool UWeaponLogicComponent::Rangefinder(const FTransform& StartTransform, FHitResult& OutHit)
{
	bool bDidHit = false;
	//bDidHit = UWeaponFunctions::PerformWeaponLineTrace(this, StartTransform, OutHit);
	//WeaponSystem.BaseWeaponSystem.EquippedWeaponState.RaycastData.RangefinderData = OutHit;

	return bDidHit;
}

void UWeaponLogicComponent::UpdateProjectileAimDirection()
{
	//FVector MuzzleLocation = UWeaponFunctions::GetMuzzleTransform("Muzzle", WeaponSystem.WeaponMeshes[WeaponSystem.BaseWeaponSystem.EquippedWeaponState.CurrentWeaponIndex]).GetLocation();
	//FVector RawAimDir = UWeaponFunctions::CalculateAimDirection(WeaponSystem.BaseWeaponSystem.EquippedWeaponState.RaycastData.RangefinderData, MuzzleLocation);
	//WeaponSystem.BaseWeaponSystem.EquippedWeaponState.RaycastData.MuzzleAimDirections[0] = RawAimDir;
}

void UWeaponLogicComponent::StartFire()
{
	FWeapon_Runtime& CurrentWeapon = *GetCurrentWeaponRuntime();
	if (CurrentWeapon.WeaponState.canFire)
	{
		CurrentWeapon.WeaponState.isFiring = true;
		switch (CurrentWeapon.WeaponState.CurrentFireMode)
		{
			case EFireMode::Single:
				break;
			case EFireMode::Burst:
				break;
			case EFireMode::Auto:
				if (!GetWorld()->GetTimerManager().IsTimerActive(TimerHandle_AutoFire))
				{
					FireWeapon();	//fire weapon immediately AND THEN fire rate every shot after
					float FireRate = UWeaponFunctions::GetFireRate(StaticWeaponDataCache[WeaponSystem.BaseWeaponSystem.EquippedWeaponState.CurrentWeaponIndex]->WeaponFirePerformance.RateOfFire);
					GetWorld()->GetTimerManager().SetTimer(TimerHandle_AutoFire, this, &UWeaponLogicComponent::FireWeapon, FireRate, true);	//looping
				}
				break;
		}
	}
	else
	{
		if (CurrentWeapon.WeaponState.CurrentAmmoinMag == 0)
		{
			DryFire();
		}
	}
}

void UWeaponLogicComponent::CeaseFire()
{
	FWeapon_Runtime& CurrentWeapon = *GetCurrentWeaponRuntime();
	if (GetWorld()->GetTimerManager().IsTimerActive(TimerHandle_AutoFire))
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle_AutoFire);
	}
	CurrentWeapon.WeaponState.isFiring = false;
}

void UWeaponLogicComponent::DryFire()
{
	//play whatever dryfire sound and animation
}

void UWeaponLogicComponent::FireWeapon()
{
	const FBaseWeaponData* StaticWeaponData = GetCurrentWeaponStaticData();
	FWeapon_Runtime& CurrentWeapon = *GetCurrentWeaponRuntime();	
	if (!CurrentWeapon.WeaponState.canFire)
	{
		return;
	}
	FTransform MuzzleTransform;
	UpdateProjectileAimDirection();
	//switch (StaticWeaponData->WeaponFirePerformance.WeaponFireType)
	//{
		//case EWeaponFireType::SimProjectile:
			//const FProjectileData& ProjectileData = *DataManager->GetProjectileDataRow(StaticWeaponData->WeaponFirePerformance.MunitionID);
			//MuzzleTransform = UWeaponFunctions::GetMuzzleTransform("Muzzle", WeaponSystem.WeaponMeshes[WeaponSystem.BaseWeaponSystem.EquippedWeaponState.CurrentWeaponIndex]);
			//UWeaponFunctions::CreateSimProjectile(MuzzleTransform.GetLocation(), StaticWeaponData->WeaponPerformance.MuzzleVelocity, ProjectileData.ProjectileFlightPlan[0].GuidanceParams.GravityScale, WeaponSystem.BaseWeaponSystem.EquippedWeaponState.RaycastData.MuzzleAimDirections[0], ProjectileManager);
			//break;
	//}
	//handle/manage ammo
}

FName UWeaponLogicComponent::GetSocketNameForSlot(EAttachmentSlot Slot)
{
	switch (Slot)
	{
		case EAttachmentSlot::Optic:       
			return TEXT("S_Optic");
		case EAttachmentSlot::Muzzle:			
			return TEXT("S_Muzzle");
		case EAttachmentSlot::Underbarrel:		
			return TEXT("S_Underbarrel");
		case EAttachmentSlot::SideRailLeft:	    
			return TEXT("S_Rail_L");
		case EAttachmentSlot::SideRailRight:   
			return TEXT("S_Rail_R");
		case EAttachmentSlot::Magazine:    
			return TEXT("S_Mag");
		case EAttachmentSlot::Stock:       
			return TEXT("S_Stock");
	}
	return NAME_None;
}

UDataManagerSubsystem* UWeaponLogicComponent::GetDataManager()
{
	return GetWorld()->GetGameInstance()->GetSubsystem<UDataManagerSubsystem>();
}

FWeapon_Runtime* UWeaponLogicComponent::GetCurrentWeaponRuntime()
{
	FWeapon_Runtime* CurrentWeapon = &WeaponSystem.BaseWeaponSystem.Weapons[WeaponSystem.BaseWeaponSystem.EquippedWeaponState.CurrentWeaponIndex];
	return CurrentWeapon;
}

const FBaseWeaponData* UWeaponLogicComponent::GetCurrentWeaponStaticData() const
{
	const FBaseWeaponData* StaticWeaponData = StaticWeaponDataCache[WeaponSystem.BaseWeaponSystem.EquippedWeaponState.CurrentWeaponIndex];
	return StaticWeaponData;
}

