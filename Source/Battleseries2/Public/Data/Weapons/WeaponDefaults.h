#pragma once

#include "CoreMinimal.h"
#include "MetasoundSource.h"
#include "Engine/DataAsset.h"
#include "WeaponDefaults.generated.h"

//global defaults for weapons

USTRUCT(BlueprintType)
struct FWeaponDefaults
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UMetaSoundSource> DefaultWeaponMetaSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FText> WeaponSlotNames;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ScopeCameraFOV = 90.0f;
};

UCLASS(BlueprintType)
class BATTLESERIES2_API UDA_WeaponDefaults : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FWeaponDefaults WeaponDefaults;
};