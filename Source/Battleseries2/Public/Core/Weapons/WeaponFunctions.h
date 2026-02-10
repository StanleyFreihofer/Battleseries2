#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "WeaponFunctions.generated.h"
class UProjectilePoolSubsystem;
struct FBaseWeaponData;
struct FWeapon_Runtime;
struct FProjectile_Runtime;

UCLASS()
class BATTLESERIES2_API UWeaponFunctions : public UObject
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static float GetFireRate(float RateOfFire);
	UFUNCTION()
	static FTransform GetMuzzleTransform(FName MuzzleSocketName, TWeakObjectPtr<USkeletalMeshComponent> SocketMesh);
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static FVector CalculateAimDirection(FHitResult TraceData, FVector MuzzleLocation);
	UFUNCTION(BlueprintCallable)
	static bool PerformWeaponLineTrace(const UObject* WorldContextObject, const FTransform& StartTransform, FHitResult& OutHit, TArray<AActor*> ActorsToIgnore);
	UFUNCTION(BlueprintCallable)
	static int32 UpdateCurrentAmmoInMag(FWeapon_Runtime& CurrentWeapon, int32 AmmoDelta, int32 MagSize);
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static void CalculateReload(int32 MagSize, int32 CAM, int32 CRA, int32& OutCAM, int32& OutCRA);
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static int32 UpdateWeaponIndex(TArray<FWeapon_Runtime> Weapons, int32 WeaponIndex);
	UFUNCTION(BlueprintCallable)
	static FProjectile_Runtime CreateSimProjectile(FName ProjectileID, class APlayerState* InstigatorPlayerState, FVector MuzzleLocation, float MuzzleSpeed, float GravityScale, FVector AimDirection, float BaseDamage, UCurveFloat* DamageDropoffCurve, UProjectilePoolSubsystem* ProjectileSubsystem);
	UFUNCTION(BlueprintCallable)
	static void Debug_ProjectilePath(const UObject* WorldContextObject, FVector MuzzleLocation, FHitResult HitResult);
};
