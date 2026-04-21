#include "Core/Weapons/WeaponFunctions.h"
#include "Data/Runtime/WeaponTypes.h"
#include "Data/Runtime/ProjectileTypes.h"
#include "Utilities/ProjectilePoolSubsystem.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"

float UWeaponFunctions::GetFireRate(float RateOfFire)
{
	RateOfFire = 60 / RateOfFire;
	return RateOfFire;
}

FTransform UWeaponFunctions::GetMuzzleTransform(FName MuzzleSocketName, TWeakObjectPtr<USkeletalMeshComponent> SocketMesh)
{
	FTransform SocketTransform;
	SocketTransform = SocketMesh->GetSocketTransform(MuzzleSocketName, RTS_World);
	return SocketTransform;
}

FVector UWeaponFunctions::CalculateAimDirection(FHitResult TraceData, FVector MuzzleLocation)
{
	FVector TargetPoint = TraceData.bBlockingHit ? TraceData.ImpactPoint : TraceData.TraceEnd;
	FVector RawAimDirection = (TargetPoint - MuzzleLocation).GetSafeNormal();
	return RawAimDirection;
}

bool UWeaponFunctions::PerformWeaponLineTrace(const UObject* WorldContextObject, const FTransform& StartTransform, FHitResult& OutHit, TArray<AActor*> ActorsToIgnore)
{
	//if calling from any actor, WorldContextObject = this/self
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	FVector Startpoint = StartTransform.GetLocation();
	FVector GetRotationXVector = StartTransform.GetRotation().Rotator().Vector();
	FVector Endpoint = GetRotationXVector * 50000.0f + Startpoint; 

	FCollisionQueryParams Params;
	Params.AddIgnoredActors(ActorsToIgnore);
	bool bDidHit = World->LineTraceSingleByChannel(OutHit, Startpoint, Endpoint, ECC_Visibility, Params);
	//DrawDebugLine(World, Startpoint, Endpoint, bDidHit ? FColor::Green : FColor::Red, false, 1.f, 0, 1.f);
	return bDidHit;
}

bool UWeaponFunctions::PerformWeaponSphereTrace(const UObject* WorldContextObject, const FTransform& StartTransform, FHitResult& OutHit, TArray<AActor*> ActorsToIgnore, float Radius)
{
	//if calling from any actor, WorldContextObject = this/self
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	FVector Startpoint = StartTransform.GetLocation();
	FVector GetRotationXVector = StartTransform.GetRotation().Rotator().Vector();
	FVector Endpoint = GetRotationXVector * 50000.0f + Startpoint;

	FCollisionQueryParams Params;
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(Radius);
	Params.AddIgnoredActors(ActorsToIgnore);
	//bool bDidHit = World->SweepSingleByChannel(OutHit, Startpoint, Endpoint, FQuat::Identity, ECC_Visibility, SphereShape, Params);

	
	return UKismetSystemLibrary::SphereTraceSingle(
		WorldContextObject,
		Startpoint,
		Endpoint,
		Radius,
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		false,              // bTraceComplex
		ActorsToIgnore,
		EDrawDebugTrace::None,
		OutHit,
		true,               // bIgnoreSelf
		FLinearColor::Red,  // Trace Color
		FLinearColor::Green,// Hit Color
		0.0f                // Draw Time
	);
	
	//return bDidHit;
}

int32 UWeaponFunctions::UpdateCurrentAmmoInMag(FWeapon_Runtime& CurrentWeapon, int32 AmmoDelta, int32 MagSize)
{
	//can be used for firing or resupplying logic
	CurrentWeapon.WeaponState.CurrentAmmoinMag = FMath::Clamp(CurrentWeapon.WeaponState.CurrentAmmoinMag + AmmoDelta, 0, MagSize);
	if (CurrentWeapon.WeaponState.CurrentAmmoinMag == 0)
	{
		CurrentWeapon.WeaponState.canFire = false;
		//CeaseFire();
		//DryFire();
	}
	UE_LOG(LogTemp, Warning, TEXT("WeaponFunction::UpdateCurrentAmmoInMag] CAM = %d"), CurrentWeapon.WeaponState.CurrentAmmoinMag);
	return CurrentWeapon.WeaponState.CurrentAmmoinMag;
}

void UWeaponFunctions::CalculateReload(int32 MagSize, int32 CAM, int32 CRA, int32& OutCAM, int32& OutCRA)
{
	//need max reserve ammo input?
	int32 BulletsFiredFromMag = MagSize - CAM;
	int32 BulletsToLoad = FMath::Min(BulletsFiredFromMag, CRA);
	OutCRA = CRA - BulletsToLoad;
	OutCAM = CAM + BulletsToLoad;
}

int32 UWeaponFunctions::UpdateWeaponIndex(TArray<FWeapon_Runtime> Weapons, int32 CurrentWeaponIndex)
{
	int32 NewWeaponIndex = CurrentWeaponIndex++;
	if (!Weapons.IsValidIndex(NewWeaponIndex))
	{
		NewWeaponIndex = 0;
	}
	return NewWeaponIndex;
}

FSimProjectile_Runtime UWeaponFunctions::CreateSimProjectile(FName MunitionID, class APlayerState* InstigatorPlayerState, FVector MuzzleLocation, float MuzzleSpeed, float GravityScale, FVector AimDirection, float BaseDamage, UCurveFloat* DamageDropoffCurve, UProjectilePoolSubsystem* ProjectileSubsystem)
{
	FSimProjectile_Runtime NewSimulatedProjectile = FSimProjectile_Runtime();
	NewSimulatedProjectile.MunitionID = MunitionID;
	NewSimulatedProjectile.FireOrigin = MuzzleLocation;
	NewSimulatedProjectile.CurrentLocation = MuzzleLocation;
	NewSimulatedProjectile.CurrentVelocity = AimDirection * MuzzleSpeed;
	NewSimulatedProjectile.GravityScale = GravityScale;
	NewSimulatedProjectile.BaseDamage = BaseDamage;
	NewSimulatedProjectile.DamageCurve = DamageDropoffCurve;
	ProjectileSubsystem->AddNewSimProjectile(NewSimulatedProjectile);
	return NewSimulatedProjectile;
}

void UWeaponFunctions::Debug_ProjectilePath(const UObject* WorldContextObject, FVector MuzzleLocation, FHitResult HitResult)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	// DEBUG: Draw the convergence line
	if (World)
	{
		// 1. Draw a line from the muzzle to the world hit point
		// If the line hits the crosshair, your convergence math is perfect.
		DrawDebugLine(World, MuzzleLocation, HitResult.ImpactPoint, FColor::Red, false, -1.f, 0, 1.0f);

		// 2. Draw a small sphere at the muzzle to verify socket location
		DrawDebugSphere(World, MuzzleLocation, 10.f, 8, FColor::Yellow, false, -1.f);

		// 3. Draw a point at the Rangefinder impact (the Target)
		DrawDebugPoint(World, HitResult.ImpactPoint, 15.f, FColor::Orange, false, -1.f);
	}
}
