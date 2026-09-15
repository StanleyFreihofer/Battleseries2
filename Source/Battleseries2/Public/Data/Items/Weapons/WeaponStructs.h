#pragma once

#include "CoreMinimal.h"
#include "Data/Core/CoreTypes.h"
#include "Data/Items/Weapons/WeaponEnums.h"
#include "WeaponStructs.generated.h"

USTRUCT(BlueprintType)
struct FWeaponStatModifierData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FStatModifierData Modifier = FStatModifierData();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "this only matters if EffectValueMode isn't FlatValue", EditCondition = "Modifier.EffectValueMode == EEffectValueMode::MultipleOfStat", EditConditionHides))
	EWeaponStat ReferenceState = EWeaponStat::MagSize;
};