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
	TObjectPtr<AVehicle_Base> CurrentVehicle = nullptr;		
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
	TWeakObjectPtr<AActor> HitInteractable = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "State")
	FTimerHandle InteractTimer = FTimerHandle();
};

USTRUCT(BlueprintType)
struct FCharacterStanceState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "State")
	ECharacterStance CurrentStance = ECharacterStance::Standing;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "State")
	FTimerHandle StanceTransitionTimer = FTimerHandle();
};

USTRUCT(BlueprintType)
struct FCharacterMovementState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "State")
	ECharacterMovementMode CurrentMovementMode = ECharacterMovementMode::Idle;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "State")
	bool canSprint = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "State")
	float CurrentStamina = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "State")
	FTimerHandle SprintTimer = FTimerHandle();
};

USTRUCT(BlueprintType)
struct FCharacterState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FCharacterMovementState CharacterMovementState = FCharacterMovementState();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "State")
	FCharacterStanceState CharacterStanceState = FCharacterStanceState();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "State")
	FCharacterVehicleState CharacterVehicleState = FCharacterVehicleState();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "State")
	FInteractionState InteractionState = FInteractionState();

	//current controlled projectile?

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	FVector2D CurrentHeadDelta = FVector2D();
};
