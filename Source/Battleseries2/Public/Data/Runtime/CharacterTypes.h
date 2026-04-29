#pragma once

#include "CoreMinimal.h"
#include "Data/Characters/CharacterEnums.h"
#include "CharacterTypes.generated.h"
class AVehicle_Base;		
class UUW_HUD_Vehicle_Base;

USTRUCT(BlueprintType)
struct FCharacterVehicleState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	bool inVehicle = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	AVehicle_Base* CurrentVehicle = nullptr;		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	bool isFreeLooking = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	int32 LSI = -1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	int32 CSI = -1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	int32 NSI = -1;
};

USTRUCT(BlueprintType)
struct FInteractionState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	AActor* HitInteractable = nullptr;

	//interacttimer
};

USTRUCT(BlueprintType)
struct FCharacterState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	ECharacterCurrentStance CurrentStance = ECharacterCurrentStance::Standing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	FCharacterVehicleState CharacterVehicleState = FCharacterVehicleState();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	FVector2D CurrentHeadDelta = FVector2D();
};
