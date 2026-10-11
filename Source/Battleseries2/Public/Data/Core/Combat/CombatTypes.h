#pragma once

#include "CoreMinimal.h"
#include "CombatTypes.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHealthChanged);

USTRUCT(BlueprintType)
struct FHealthState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float CurrentHealth = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)			//used both for delay and tick
	FTimerHandle RegenTimer = FTimerHandle();
};

USTRUCT(BlueprintType)
struct FVehicleHealthState
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FHealthState BaseHealthState = FHealthState();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bIsImmobilized = false;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UStaticMeshComponent> DestroyedMesh = nullptr;
};

//combat state?
//is lazed/designated?
//current faction