#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Data/Core/CoreTypes.h"
#include "CoreDefaults.generated.h"

UCLASS(BlueprintType)
class BATTLESERIES2_API UDA_CoreDefaults : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<ECoreType, FCoreTypeEnumDefinition>  CoreTypeDefinitions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<ECoreItemType, FCoreTypeEnumDefinition> CoreItemTypeDefinitions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<ECharacterItemType, FCoreTypeEnumDefinition> CharacterItemTypeDefinitions;

	//UPROPERTY(EditAnywhere, BlueprintReadOnly)
	//TMap<EWeaponItemType, FCoreTypeEnumDefinition> WeaponItemTypeDefinitions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<EVehicleItemType, FCoreTypeEnumDefinition> VehicleItemTypeDefinitions;
};
