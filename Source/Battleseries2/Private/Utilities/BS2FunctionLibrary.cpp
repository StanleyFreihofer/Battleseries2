#include "Utilities/BS2FunctionLibrary.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "Data/Weapons/WeaponTypes.h"
#include "Data/Weapons/ProjectileTypes.h"
#include "Utilities/BS2FunctionLibrary.h"
#include "Utilities/DataManagerSubsystem.h"
#include "Utilities/ProjectilePoolSubsystem.h"
#include "Utilities/HUDSubsystem.h"
#include "Save/SaveSubsystem.h"
#include "Utilities/I_VehicleDataAccessor.h"

bool UBS2FunctionLibrary::PerformSphereTraceMulti(const UObject* WorldContextObject, const FTransform StartTransform, TArray<FHitResult>& OutHits, TArray<AActor*> ActorsToIgnore, float Radius, float Distance, bool Debug)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	FVector Startpoint = StartTransform.GetLocation();
	FVector GetRotationXVector = StartTransform.GetRotation().Rotator().Vector();
	FVector Endpoint = GetRotationXVector * Distance + Startpoint;

	FCollisionQueryParams Params;
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(Radius);
	Params.AddIgnoredActors(ActorsToIgnore);
	EDrawDebugTrace::Type DebugTrace = Debug ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;					//<--namespaced enum

	return UKismetSystemLibrary::SphereTraceMulti(
		WorldContextObject,
		Startpoint,
		Endpoint,
		Radius,
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		false,              // bTraceComplex
		ActorsToIgnore,
		DebugTrace,
		OutHits,
		true,               // bIgnoreSelf
		FLinearColor::Red,  // Trace Color
		FLinearColor::Green,// Hit Color
		0.0f                // Draw Time
	);
	return false;
}

void UBS2FunctionLibrary::ConvertNamesToVehicleTypes(const TArray<FName>& VehicleTypeNames, TArray<EVehicleType>& OutVehicleTypes)
{
	OutVehicleTypes.Empty();
	for (const FName& VehicleTypeName : VehicleTypeNames)
	{
		//FString VehicleTypeString = RowName.ToString();

		//EVehicleType VehicleType = EVehicleType::VE_None; //default/fallback
		
		int64 FoundEnum = StaticEnum<EVehicleType>()->GetValueByName(VehicleTypeName);
		if (FoundEnum != INDEX_NONE)
		{
			OutVehicleTypes.Add(static_cast<EVehicleType>(FoundEnum));
			//return FoundEnum;
		}
		
	}
}

FString UBS2FunctionLibrary::GetVehicleTypeLiteralString(EVehicleType VehicleType)
{
	// Use StaticEnum to look up the name
	UEnum* EnumPtr = StaticEnum<EVehicleType>();
	if (!EnumPtr)
	{
		return FString("Invalid");
	}

	// Get the literal enum name (e.g. "IFV", "Tank", etc.)
	return EnumPtr->GetNameStringByValue(static_cast<int64>(VehicleType));
}

UDataManagerSubsystem* UBS2FunctionLibrary::GetDataSubsystem(const UObject* WorldContextObject)
{
	return WorldContextObject->GetWorld()->GetGameInstance()->GetSubsystem<UDataManagerSubsystem>();
}

UHUDSubsystem* UBS2FunctionLibrary::GetHUDSubsystem(const UObject* WorldContextObject)
{
	return WorldContextObject->GetWorld()->GetFirstLocalPlayerFromController()->GetSubsystem<UHUDSubsystem>();
}

USaveSubsystem* UBS2FunctionLibrary::GetSaveSubsystem(const UObject* WorldContextObject)
{
	return WorldContextObject->GetWorld()->GetGameInstance()->GetSubsystem<USaveSubsystem>();
}

UProjectilePoolSubsystem* UBS2FunctionLibrary::GetProjectileSystem(const UObject* WorldContextObject)
{
	return WorldContextObject->GetWorld()->GetSubsystem<UProjectilePoolSubsystem>();
}

IVehicleDataAccessor* UBS2FunctionLibrary::GetVehicleAccessor(AActor* TargetActor)
{
	return Cast<IVehicleDataAccessor>(TargetActor);
}

float UBS2FunctionLibrary::GetFireRate(float RateOfFire)
{
	RateOfFire = 60 / RateOfFire;
	return RateOfFire;
}

bool UBS2FunctionLibrary::PerformWeaponLineTrace(const UObject* WorldContextObject, const FTransform& StartTransform, FHitResult& OutHit, TArray<AActor*> ActorsToIgnore, bool Debug)
{
	//if calling from any actor, WorldContextObject = this/self
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	FVector Startpoint = StartTransform.GetLocation();
	FVector GetRotationXVector = StartTransform.GetRotation().Rotator().Vector();
	FVector Endpoint = GetRotationXVector * 50000.0f + Startpoint;

	FCollisionQueryParams Params;
	Params.AddIgnoredActors(ActorsToIgnore);
	bool bDidHit = World->LineTraceSingleByChannel(OutHit, Startpoint, Endpoint, ECC_Visibility, Params);
	if (Debug)
	{
		DrawDebugLine(World, Startpoint, Endpoint, bDidHit ? FColor::Green : FColor::Red, false, 1.f, 0, 1.f);
	}

	return bDidHit;
}

bool UBS2FunctionLibrary::PerformWeaponSphereTrace(const UObject* WorldContextObject, const FTransform& StartTransform, FHitResult& OutHit, TArray<AActor*> ActorsToIgnore, float Radius, bool Debug)
{
	//if calling from any actor, WorldContextObject = this/self
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	FVector Startpoint = StartTransform.GetLocation();
	FVector GetRotationXVector = StartTransform.GetRotation().Rotator().Vector();
	FVector Endpoint = GetRotationXVector * 1000000.0f + Startpoint;			//(1000000 = 6 miles)

	FCollisionQueryParams Params;
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(Radius);
	Params.AddIgnoredActors(ActorsToIgnore);
	EDrawDebugTrace::Type DebugTrace = Debug ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;

	return UKismetSystemLibrary::SphereTraceSingle(
		WorldContextObject,
		Startpoint,
		Endpoint,
		Radius,
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		false,              // bTraceComplex
		ActorsToIgnore,
		DebugTrace,
		OutHit,
		true,               // bIgnoreSelf
		FLinearColor::Red,  // Trace Color
		FLinearColor::Green,// Hit Color
		0.0f                // Draw Time
	);

	//return bDidHit;
}

FTransform UBS2FunctionLibrary::GetMuzzleTransform(FName MuzzleSocketName, TWeakObjectPtr<USkeletalMeshComponent> SocketMesh)
{
	FTransform SocketTransform;
	SocketTransform = SocketMesh->GetSocketTransform(MuzzleSocketName, RTS_World);
	return SocketTransform;
}

FVector UBS2FunctionLibrary::CalculateAimDirection(FHitResult TraceData, FVector MuzzleLocation)
{
	FVector TargetPoint = TraceData.bBlockingHit ? TraceData.ImpactPoint : TraceData.TraceEnd;
	FVector RawAimDirection = (TargetPoint - MuzzleLocation).GetSafeNormal();
	return RawAimDirection;
}

FVector UBS2FunctionLibrary::GetAimDirectionFromMuzzle(FHitResult TraceData, FName MuzzleSocketName, TWeakObjectPtr<USkeletalMeshComponent> WeaponMesh)
{
	FVector MuzzleLocation = GetMuzzleTransform(MuzzleSocketName, WeaponMesh).GetLocation();
	FVector AimDirection = CalculateAimDirection(TraceData, MuzzleLocation);
	return AimDirection;
}

FVector UBS2FunctionLibrary::GetAimDirectionFromMuzzle_BP(FHitResult TraceData, FName MuzzleSocketName, USkeletalMeshComponent* WeaponMesh)
{
	return GetAimDirectionFromMuzzle(TraceData, MuzzleSocketName, TWeakObjectPtr<USkeletalMeshComponent>(WeaponMesh));
}

FSimProjectile_Runtime UBS2FunctionLibrary::CreateSimProjectile(FName MunitionID, class APlayerState* InstigatorPlayerState, FVector MuzzleLocation, float MuzzleSpeed, float GravityScale, FVector AimDirection, float BaseDamage, UCurveFloat* DamageDropoffCurve, UProjectilePoolSubsystem* ProjectileSubsystem)
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

void UBS2FunctionLibrary::CalculateReload(int32 MagSize, int32 CAM, int32 CRA, int32& OutCAM, int32& OutCRA)
{
	//need max reserve ammo input?
	int32 BulletsFiredFromMag = MagSize - CAM;
	int32 BulletsToLoad = FMath::Min(BulletsFiredFromMag, CRA);
	OutCRA = CRA - BulletsToLoad;
	OutCAM = CAM + BulletsToLoad;
}

int32 UBS2FunctionLibrary::UpdateCurrentAmmoInMag(FWeapon_Runtime& CurrentWeapon, int32 AmmoDelta, int32 MagSize)
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

void UBS2FunctionLibrary::UpdateWeaponIndex(TArray<FWeapon_Runtime> Weapons, int32 InCurrentWeaponIndex, int32& OutNewWeaponIndex)
{
	OutNewWeaponIndex = (InCurrentWeaponIndex + 1) % Weapons.Num();
}

void UBS2FunctionLibrary::HandleUpdateOptic(float inDefaultFOV, float inOpticMagnfication, float& OutOpticFOV, FPostProcessSettings inPostProcessData, FPostProcessSettings& OutPostProcessSettings, float& OutPostProcessWeight)
{
	//Handles both Post Process (thermal, night vision, etc.) and FOV changes for optics
	if (inPostProcessData.WeightedBlendables.Array.Num() > 0)
	{
		OutPostProcessSettings = inPostProcessData;
		OutPostProcessWeight = 1.0f;
	}
	else
	{
		OutPostProcessSettings = FPostProcessSettings();
		OutPostProcessWeight = 0.0f;
	}
	OutOpticFOV = inDefaultFOV / inOpticMagnfication;
}

void UBS2FunctionLibrary::UpdateOpticIndex(int32 TotalOptics, int32& CurrentOpticIndex)
{
	int32 NewOpticIndex = (CurrentOpticIndex + 1) % TotalOptics;
	CurrentOpticIndex = NewOpticIndex;
}


