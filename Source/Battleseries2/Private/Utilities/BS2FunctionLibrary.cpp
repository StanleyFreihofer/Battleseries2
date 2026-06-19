#include "Utilities/BS2FunctionLibrary.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "Utilities/BS2FunctionLibrary.h"
#include "Utilities/DataManagerSubsystem.h"
#include "Utilities/ProjectilePoolSubsystem.h"
#include "Utilities/HUDSubsystem.h"
#include "Save/SaveSubsystem.h"
#include "Utilities/I_VehicleDataAccessor.h"

bool UBS2FunctionLibrary::PerformSphereTraceMulti(const UObject* WorldContextObject, const FTransform StartTransform, TArray<FHitResult>& OutHits, TArray<AActor*> ActorsToIgnore, float Radius, float Distance)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	FVector Startpoint = StartTransform.GetLocation();
	FVector GetRotationXVector = StartTransform.GetRotation().Rotator().Vector();
	FVector Endpoint = GetRotationXVector * Distance + Startpoint;

	FCollisionQueryParams Params;
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(Radius);
	Params.AddIgnoredActors(ActorsToIgnore);

	return UKismetSystemLibrary::SphereTraceMulti(
		WorldContextObject,
		Startpoint,
		Endpoint,
		Radius,
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		false,              // bTraceComplex
		ActorsToIgnore,
		EDrawDebugTrace::ForOneFrame,
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



