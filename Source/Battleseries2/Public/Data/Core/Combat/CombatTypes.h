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

	//UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	//bool bIsDead = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)			//used both for delay and tick
	FTimerHandle RegenTimer = FTimerHandle();

	//UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	//FTimerHandle RegenTickTimer = FTimerHandle();
};

USTRUCT(BlueprintType)
struct FVehicleHealthState
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FHealthState BaseHealthState = FHealthState();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bIsImmobilized = false;
};