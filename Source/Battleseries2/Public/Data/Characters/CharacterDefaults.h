#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "CharacterDefaults.generated.h"

class UUW_HUD_Status_Base;

/**
 *	default data for characters during gameplay
 *  can also be seen as defaults for parameters during gameplay
 * controller defaults?
 */

UCLASS(BlueprintType)
class BATTLESERIES2_API UDA_CharacterDefaults : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TSoftObjectPtr<UInputMappingContext> DefaultIMC = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TSoftObjectPtr<UInputMappingContext> DefaultGameplayIMC = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	TSubclassOf<UUW_HUD_Status_Base> StatusHUDClass = nullptr;

	//interaction range
	//minimap HUD class (gameplay hud)
};