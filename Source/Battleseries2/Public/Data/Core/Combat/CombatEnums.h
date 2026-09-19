#pragma once

#include "CoreMinimal.h"
#include "CombatEnums.generated.h"

/**
 * health, armor specific enum
 */

UENUM(BlueprintType)
enum class EArmorType : uint8
{
	Infantry			UMETA(DisplayName = "Infantry"),
	LightArmor			UMETA(DisplayName = "Light Armor"),			//jeeps and whatnot
	MediumArmor			UMETA(DisplayName = "Medium Armor"),		//IFV's
	HeavyArmor			UMETA(DisplayName = "Heavy Armor")			//Tanks
	
	//aircraft
};

UENUM(BlueprintType)
enum class EDamageCategory : uint8
{
	Ballistic,
	Explosive,
	Fire
};
