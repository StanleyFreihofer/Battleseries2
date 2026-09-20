#pragma once

#include "CoreMinimal.h"
#include "Data/Core/Combat/CombatEnums.h"
#include "UObject/Interface.h"
#include "I_Damageable.generated.h"

UINTERFACE(MinimalAPI)
class UDamageable : public UInterface
{
	GENERATED_BODY()
};

class BATTLESERIES2_API IDamageable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	EArmorType GetArmorType();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	float GetHitZoneMultiplier(FName BoneName, FVector HitLocation);		//armor side, headshot multipliers, etc
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Kill();
	
	
};