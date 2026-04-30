#pragma once

#include "CoreMinimal.h"
#include "CharacterEnums.generated.h"

UENUM(BlueprintType)
enum class ECharacterCurrentStance : uint8
{
	Standing			UMETA(DisplayName = "Standing"),
	Crouching			UMETA(DisplayName = "Crouching"),
	Proning				UMETA(DisplayName = "Proning"),
	Sitting				UMETA(DisplayName = "Sitting")
};

UENUM(BlueprintType)
enum class ECharacterMovementMode : uint8
{
	Walking				UMETA(DisplayName = "Walking"),
	Sprinting			UMETA(DisplayName = "Sprinting"),
};

UENUM(BlueprintType)
enum class EControlRotationMethod : uint8
{
	Default				UMETA(DisplayName = "Look Around Default"),
	Freelook			UMETA(DisplayName = "Freelook/Look Around w/Head"),
	None				UMETA(DisplayName = "None", ToolTip = "Cannot look around at all")
};

//swimming?
//parachuting?
//free falling?
//tac sprinting?
//sliding?
//jumping
