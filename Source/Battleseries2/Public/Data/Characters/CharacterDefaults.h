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

USTRUCT(BlueprintType)
struct FCharacterMovementDefaults
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxAcceleration = 3072.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxWalkSpeed = 830.0f;				//373.5?

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxWalkSpeedCrouched = 280.0f;
};

USTRUCT(BlueprintType)
struct FCharacterMovementData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FCharacterMovementDefaults CharacterMovementDefaults = FCharacterMovementDefaults();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool canJump = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool canSprint = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxSprintSpeed = 647.4f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool canCrouch = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool canCrouchSprint = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxCrouchSprintSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool canProne = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool canProneSprint = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxProneSprintSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float StaminaDuration = 0.0f;
};

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Default Trace")
	float TraceDistance = 75000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	float InteractionDistance = 75.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	struct FCharacterMovementData CharacterMovementData = FCharacterMovementData();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stances")
	FVector ProneFPHeight = FVector::ZeroVector;
	//minimap HUD class (gameplay hud)
	//spotting range?
};