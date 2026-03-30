#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/WidgetComponent.h"
#include "VehicleTypes.generated.h"

USTRUCT(BlueprintType)
struct FOpticState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)		//the optics available to cycle through (first option is default) (OpticIDs)
	TArray<FName> CurrentAvailableOptics;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentOpticIndex = 0;
};

USTRUCT(BlueprintType)
struct FSeatState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)			//occupant name string instead?
	bool isOccupied = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool UpdateHUD = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UWidgetComponent* SeatHUDComponent = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	USpringArmComponent* DefaultSpringArm = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UCameraComponent* DefaultCamera = nullptr;		//the safety, the camera that should be there no matter what weapon or whatever is equipped

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	USpringArmComponent* ActiveSpringArm = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UCameraComponent* ActiveCamera = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FOpticState OpticState = FOpticState();

	//current POV mode?

};

USTRUCT(BlueprintType)
struct FGroundVehicleState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentSpeed = 0.0f;
	//current steering angle
};

USTRUCT(BlueprintType)
struct FAircraftState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLandingGearIsDown = true;
};

USTRUCT(BlueprintType)
struct FGenericVehicleState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	class UAudioComponent* EngineAudioComponent = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	class UAudioComponent* HornAudioComponent = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CurrentCamo = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool LoadoutApplied = false;			//<--- change to string, denote WHO's loadout it is?
};

USTRUCT(BlueprintType)
struct FVehicleCurrentState
{
	GENERATED_BODY()
	//health?
	//some sort of string/identifier of who/what loadout its carrying (might be good for comparing if different people hop into the main seat)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FSeatState> SeatStates;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGenericVehicleState GenericVehicleState = FGenericVehicleState();
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGroundVehicleState GroundVehicleState = FGroundVehicleState();
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FAircraftState AircraftState = FAircraftState();
	//combat state (is lazed/designated?)
	//armor?
	//countermeasure?
	//upgrade?
	//camo?
	//current faction?
};



