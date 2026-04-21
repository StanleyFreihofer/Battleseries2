
#include "Core/Weapons/WeaponLogicComponent.h"
#include "Core/Weapons/WeaponFunctions.h"
#include "Data/Runtime/ProjectileTypes.h"
#include "Data/Weapons/Data_Weapon.h"
#include "Data/Weapons/Data_Projectile.h"

UWeaponLogicComponent::UWeaponLogicComponent()
{

}

void UWeaponLogicComponent::BeginPlay()
{
	Super::BeginPlay();
	DataManager = GetWorld()->GetGameInstance()->GetSubsystem<UDataManagerSubsystem>();
	ProjectileManager = GetWorld()->GetSubsystem<UProjectilePoolSubsystem>();
}

void UWeaponLogicComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	//the rangefinder gets called on tick by the owner in its context and sets the weaponsystem.rangefinder data
	//that data (OutHit) is then used here to calculate the projectile spawn transform (how projectile move towards COS is done)
	//UpdateProjectileAimDirection();
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
	FVector MuzzleLocation = UWeaponFunctions::GetMuzzleTransform("Muzzle", WeaponSystem.WeaponMeshes[WeaponSystem.BaseWeaponSystem.EquippedWeaponState.CurrentWeaponIndex]).GetLocation();
	FVector RawAimDir = UWeaponFunctions::CalculateAimDirection(WeaponSystem.BaseWeaponSystem.EquippedWeaponState.RaycastData.RangefinderData, MuzzleLocation);
	WeaponSystem.BaseWeaponSystem.EquippedWeaponState.RaycastData.MuzzleAimDirections[0] = RawAimDir;
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

