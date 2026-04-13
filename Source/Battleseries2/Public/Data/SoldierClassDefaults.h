#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Data/Core/CoreEnums.h"
#include "Data/Weapons/WeaponEnums.h"
#include "SoldierClassDefaults.generated.h"

USTRUCT(BlueprintType)
struct FSoldierClassDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FText DisplayName = FText::GetEmpty();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FText Description = FText::GetEmpty();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TSoftObjectPtr<UTexture2D> ClassIcon = nullptr;               //minimap?

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TArray<EWeaponType> AvailableWeaponCategories;             //first 1 in the array should be that class's main weapon type

    //available gadgets
};

UCLASS(BlueprintType)
class BATTLESERIES2_API UDA_SoldierClassDefaults : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TMap<EClassType, FSoldierClassDefinition> SoldierClassDefinitions;
};
