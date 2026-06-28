
#include "Core/Weapons/WeaponLogicComponent.h"
#include "Core/Weapons/WeaponFunctions.h"
#include "Character_Base.h"
#include "Data/Core/CoreTypes.h"
#include "Data/Weapons/ProjectileTypes.h"
#include "Data/Weapons/Data_Weapon.h"
#include "Data/Weapons/Data_InfantryWeapon.h"
#include "Data/Weapons/Data_WeaponAttachments.h"
#include "Data/Weapons/Data_Projectile.h"
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

	for (int32 i = 0; i < Weapons.Num(); i++)
	{
		Init_Weapon(Weapons[i], i, WeaponLoadouts[i]);
	}
}

void UWeaponLogicComponent::Init_Weapon(FName WeaponID, int32 WeaponIndex, FPlayerLoadoutConfig_Weapon WeaponLoadout)
{
	//Init WeaponSlot		Init Weapon
	FInfantryWeaponState NewFPState;
	Init_WeaponMesh(NewFPState.WeaponMesh);
	UpdateWeaponMesh(WeaponID, NewFPState.WeaponMesh);
	NewFPState.WeaponMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	WeaponSystem.InfantryWeaponSystem.WeaponState_FP[WeaponIndex] = NewFPState;
	WeaponSystem.InfantryWeaponSystem.WeaponState_FP[WeaponIndex].WeaponMesh->AttachToComponent(Cast<ACharacter_Base>(GetOwner())->FPArms, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, true), FName("Socket_M4A1"));

	WeaponSystem.BaseWeaponSystem.Weapons[WeaponIndex].WeaponID = WeaponID;
	StaticWeaponDataCache[WeaponIndex] = UBS2FunctionLibrary::GetDataSubsystem(this)->GetInfantryWeaponDataRow(WeaponID);

	//apply saved attachments
	const FPlayerLoadoutConfig_Weapon& CustomWeapon = WeaponLoadout;
	ApplyAttachments(CustomWeapon, WeaponIndex);
	//NewWeapon->SetHiddenInGame(true);
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

void UWeaponLogicComponent::UpdateWeaponCollision(ECollisionChannel CollisionChannel, ECollisionResponse CollisionResponse)
{
	WeaponSystem.InfantryWeaponSystem.WeaponState_FP[WeaponSystem.BaseWeaponSystem.EquippedWeaponState.CurrentWeaponIndex].WeaponMesh->SetCollisionResponseToChannel(CollisionChannel, CollisionResponse);
	for (auto& Attachment : WeaponSystem.InfantryWeaponSystem.WeaponState_FP[WeaponSystem.BaseWeaponSystem.EquippedWeaponState.CurrentWeaponIndex].WeaponAttachmentStates)
	{
		Attachment.Value.SpawnedAttachment.Get()->SetCollisionResponseToChannel(CollisionChannel, CollisionResponse);
	}
	//WeaponSystem.InfantryWeaponSystem.WeaponState_TP[WeaponSystem.BaseWeaponSystem.EquippedWeaponState.CurrentWeaponIndex].WeaponMesh->SetCollisionResponseToChannel(CollisionChannel, CollisionResponse);
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

float UWeaponLogicComponent::CalculateFinalStatValue(float BaseValue, TArray<FStatModifierData>& ModifierArray)
{
	float CurrentValue = BaseValue;
	for (FStatModifierData& Modifier : ModifierArray)
	{
		CurrentValue = Modifier.ApplyToValue(CurrentValue);
	}
	return CurrentValue;
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

FTransform UWeaponLogicComponent::GetSightTransform()
{
	FTransform SightTransform = WeaponSystem.InfantryWeaponSystem.WeaponState_FP[GetCWI()].WeaponMesh->GetSocketTransform(FName("Aimpoint"), ERelativeTransformSpace::RTS_Component);
	FInfantryWeaponState& InfantryWeaponState = WeaponSystem.InfantryWeaponSystem.WeaponState_FP[GetCWI()];
	float VerticalAimpointOffset = 0.0f;
	FName AttachmentID = NAME_None;
	if (FWeaponAttachmentState* OpticState = InfantryWeaponState.WeaponAttachmentStates.Find(EAttachmentSlot::Optic))
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
	if (FWeaponAttachmentState* OpticState = InfantryWeaponState.WeaponAttachmentStates.Find(EAttachmentSlot::Optic))
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
	GetAllAttachmentModifierDataOfTypeForWeapon(WeaponAttachmentStates, EStatToAffect::ADS_Speed, ADSModifiers);
	float CurrentAimInSpeed = CalculateFinalStatValue(DefaultAimInSpeed, ADSModifiers);
	AimInSpeed = CurrentAimInSpeed;
	AimOutSpeed = DefaultAimOutSpeed;
}

void UWeaponLogicComponent::GetAllAttachmentModifierDataOfTypeForWeapon(TMap<EAttachmentSlot, FWeaponAttachmentState>& WeaponAttachmentStates, EStatToAffect StatType, TArray<FStatModifierData>& OutAttachmentModifierData)
{
	//consider doing this on start or pickup of a weapon and gather/cache a source of truth/data struct for THE CURRENT STATS of the weapon with all of its attachments
	//that way we may not have to do expensive lookups
	for (auto& AttachmentSlot : WeaponAttachmentStates)
	{
		FWeaponAttachmentState& AttachmentState = AttachmentSlot.Value;
		FName& AttachmentID = AttachmentState.BaseAttachmentState.AttachmentID;
		const FStatModifierData* NewAttachmentModifierData = UBS2FunctionLibrary::GetDataSubsystem(this)->GetWeaponAttachmentDataRow(AttachmentID)->AttachmentModifiers.Find(StatType);
		if (NewAttachmentModifierData)
		{
			OutAttachmentModifierData.Add(*NewAttachmentModifierData);
		}
	}
}

FWeapon_Runtime& UWeaponLogicComponent::GetBaseWeaponState(int32 WeaponIndex)
{
	return WeaponSystem.BaseWeaponSystem.Weapons[WeaponIndex];
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

int32 UWeaponLogicComponent::GetCWI()
{
	return WeaponSystem.BaseWeaponSystem.EquippedWeaponState.CurrentWeaponIndex;
}

