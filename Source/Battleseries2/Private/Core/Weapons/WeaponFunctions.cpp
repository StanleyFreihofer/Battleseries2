#include "Core/Weapons/WeaponFunctions.h"
#include "Data/Weapons/WeaponTypes.h"
#include "Data/Weapons/ProjectileTypes.h"
#include "Utilities/ProjectilePoolSubsystem.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"



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
