#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Data/Items/Weapons/WeaponEnums.h"
#include "I_LockOnTarget.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class ULockOnTarget : public UInterface
{
	GENERATED_BODY()
};

class BATTLESERIES2_API ILockOnTarget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void LockOnTarget();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool GetIfCanLockOn(const TArray<ETargetingCategory>& TargetingCategories, EHomingCapability HomingCapability);

};