#pragma once

#include "CoreMinimal.h"
#include "Data/Core/Combat/CombatEnums.h"
#include "Data_Combat.generated.h"

/**
 * health, armor specific stuff
 */

USTRUCT(BlueprintType)
struct FHealthData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxHealth = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "seconds after last damage before regen begins"))
	float RegenDelay = 16.f;   // BF3's actual value

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "HP per second once regen begins"))
	float RegenRate = 5.f;     // BF3's actual value

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "what type of armor this is"))
	EArmorType ArmorType = EArmorType::Infantry;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "maps Bone Name to damage multiplier, use create things like vehicle armor siding, headshot multipliers, etc"))
	TMap<FName, float> HitLocationMultipliers;
	
	float CalculateHitLocationMultiplier(float Damage, FName HitBoneName) const
	{
		if (const float* LocationMultiplier = HitLocationMultipliers.Find(HitBoneName))
		{
			return Damage * (*LocationMultiplier);
		}
		return Damage;
	}
};