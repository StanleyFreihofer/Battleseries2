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
	EWeaponStat ReferenceStat = EWeaponStat::MagSize;
};

USTRUCT(BlueprintType)
struct FWeaponAnimData_ArmsReload
{
	//arms/character anims
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Actions | Montages")
	TSoftObjectPtr<UAnimMontage> ReloadWeaponMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Actions | Montages")
	TSoftObjectPtr<UAnimMontage> ReloadEmptyWeaponMontage = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transitions | Montages")
	TSoftObjectPtr<UAnimMontage> EnterReloadMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transitions | Montages")
	TSoftObjectPtr<UAnimMontage> ExitReloadMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transitions | Montages")
	TSoftObjectPtr<UAnimMontage> LoopReloadMontage = nullptr;
};

USTRUCT(BlueprintType)
struct FWeaponAnimData_Reload
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UAnimSequence> WeaponReload = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UAnimSequence> WeaponReloadEmpty = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UAnimSequence> WeaponReloadEnter = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UAnimSequence> WeaponReloadExit = nullptr;
};