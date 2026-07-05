#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "WeaponFunctions.generated.h"
class UProjectilePoolSubsystem;
struct FBaseWeaponData;
struct FWeapon_Runtime;
struct FSimProjectile_Runtime;

//BEING DEPRECIATED AND MOVED TO UBS2FunctionLibrary

UCLASS()
class BATTLESERIES2_API UWeaponFunctions : public UObject
{
	GENERATED_BODY()
	
public:






	UFUNCTION(BlueprintCallable)
	static void Debug_ProjectilePath(const UObject* WorldContextObject, FVector MuzzleLocation, FHitResult HitResult);
};
