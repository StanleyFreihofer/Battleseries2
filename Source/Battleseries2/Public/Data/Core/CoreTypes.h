#pragma once

#include "CoreMinimal.h"
#include "Data/Core/CoreEnums.h"
#include "CoreTypes.generated.h"

class UStaticMeshComponent;
class USpotLightComponent;

/**
ALL THE DIFFERENT ITEMS/THINGS THAT MAKE UP AN ENTITY (make up a character class, vehicle, weapon, etc)
alot of this is very useful/used for customization
**/

USTRUCT(BlueprintType)
struct FDecorative_Runtime
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TWeakObjectPtr<UStaticMeshComponent> DecorativeMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TWeakObjectPtr<USpotLightComponent> DecorativeSpotLight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName DecorativeAttachmentID = NAME_None;
};

USTRUCT(BlueprintType)
struct FSavedSeatLoadout
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> Weapons;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Optic = NAME_None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Upgrade = NAME_None;
};

USTRUCT(BlueprintType)
struct FPlayerLoadoutConfig_WeaponAttachment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName AttachmentID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentRailOffset = 0.0f;		//tuning value
};

#pragma region LoadoutConfigData

USTRUCT(BlueprintType)
struct FPlayerLoadoutConfig_Weapon
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EAttachmentSlot, FPlayerLoadoutConfig_WeaponAttachment> WeaponAttachments;
};

USTRUCT(BlueprintType)
struct FPlayerLoadoutConfig_Class
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> Weapons;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Gadget1 = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Gadget2 = NAME_None;
};

USTRUCT(BlueprintType)
struct FPlayerLoadoutConfig_Vehicle
{
	GENERATED_BODY()

	//Map Seat Index with array of selected weapons, etc FOR THAT SEAT (Seat Index, WeaponIDs)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<int32, FSavedSeatLoadout> SeatLoadout;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleCamo = NAME_None;
};

#pragma endregion

USTRUCT(BlueprintType)				//map this with 	EStatToAffect Stat 
struct FStatModifierData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EModifierOp Operation = EModifierOp::Multiply;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ModifierValue = 1.0f;

	float ApplyToValue(float InCurrentValue) const
	{
		switch (Operation)
		{
			case EModifierOp::Set:
				return ModifierValue;
			case EModifierOp::Add:
				return InCurrentValue + ModifierValue;
			case EModifierOp::Subtract:
				return InCurrentValue - ModifierValue;
			case EModifierOp::Multiply:
				return InCurrentValue * ModifierValue;
			case EModifierOp::Divide:
				if (!FMath::IsNearlyZero(ModifierValue))
				{
					return InCurrentValue / ModifierValue;
				}
				return InCurrentValue; // Guard against division by zero
			default:
				return InCurrentValue;
		}
	}
};


USTRUCT(BlueprintType)
struct FCoreTypeEnumDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DisplayName = FText::GetEmpty();
};






