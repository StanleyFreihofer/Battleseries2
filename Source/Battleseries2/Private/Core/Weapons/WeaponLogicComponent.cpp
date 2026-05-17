
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

void UWeaponLogicComponent::Init_WeaponLoadout(FPlayerLoadoutConfig_Class ClassLoadout, TArray<FPlayerLoadoutConfig_Weapon> WeaponLoadouts)
{
	TArray<FName>& Weapons = ClassLoadout.Weapons;
	for (int32 i = 0; i < Weapons.Num(); i++)
	{
		WeaponSystem.BaseWeaponSystem.Weapons[i].WeaponID = Weapons[i];
		FInfantryWeaponState NewFPState;

		//Create Weapon Mesh
		TWeakObjectPtr<USkeletalMeshComponent> NewWeapon = NewObject<USkeletalMeshComponent>(GetOwner());
		NewWeapon->RegisterComponent();

		//Update Mesh
		const FInfantryWeaponData& WeaponData = *GetDataManager()->GetInfantryWeaponDataRow(Weapons[i]);
		TWeakObjectPtr<USkeletalMesh> WeaponMesh = WeaponData.WeaponClassificationData.WeaponMesh.LoadSynchronous();
		NewWeapon->SetSkeletalMesh(WeaponMesh.Get());
		NewFPState.WeaponMesh = NewWeapon;

		//attach and cache
		WeaponSystem.InfantryWeaponSystem.WeaponState_FP.Add(NewFPState);
		NewWeapon->AttachToComponent(Cast<ACharacter_Base>(GetOwner())->GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepRelative, true));

		//apply saved attachments
		const FPlayerLoadoutConfig_Weapon& CustomWeapon = WeaponLoadouts[i];				
		ApplyAttachments(CustomWeapon, i);
		//NewWeapon->SetHiddenInGame(true);
	}

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

		//update attachment mesh
		const FWeaponAttachmentData& AttachmentData = *GetDataManager()->GetWeaponAttachmentDataRow(AttachmentConfig.AttachmentID);			//hardcoded for now
		TWeakObjectPtr<UStaticMesh> AttachmentMesh = AttachmentData.AttachmentClassification.AttachmentMesh.LoadSynchronous();
		const FVector& AttachmentOffset = GetAttachmentDefaultOffset(GetBaseWeaponState(WeaponIndex).WeaponID, SlotType, AttachmentConfig.AttachmentID);

		UpdateAttachment(RuntimeSlotState, AttachmentMesh, AttachmentConfig.AttachmentID, AttachmentOffset);
	}
}

void UWeaponLogicComponent::UpdateAttachment(FWeaponAttachmentState& RuntimeSlotState, TWeakObjectPtr<UStaticMesh> AttachmentMesh, FName AttachmentID, FVector Offset)
{
	RuntimeSlotState.SpawnedAttachment->SetStaticMesh(AttachmentMesh.Get());
	RuntimeSlotState.SpawnedAttachment->SetRelativeLocation(Offset);
	RuntimeSlotState.BaseAttachmentState.AttachmentID = AttachmentID;
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
		case EAttachmentSlot::FrontSight:
			return TEXT("S_FrontSight");
		case EAttachmentSlot::RearSight:
			return TEXT("S_RearSight");
		case EAttachmentSlot::Optic:       
			return TEXT("S_Optic");
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

FWeapon_Runtime& UWeaponLogicComponent::GetBaseWeaponState(int32 WeaponIndex)
{
	return WeaponSystem.BaseWeaponSystem.Weapons[WeaponIndex];
}

FVector UWeaponLogicComponent::GetAttachmentDefaultOffset(FName WeaponID, EAttachmentSlot Slot, FName AttachmentID)
{
	return GetDataManager()->GetInfantryWeaponDataRow(WeaponID)->AvailableAttachmentSlots.Find(Slot)->Attachments.Find(AttachmentID)->LocationOffset;
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

