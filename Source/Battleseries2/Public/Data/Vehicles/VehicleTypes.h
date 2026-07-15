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

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)		//the optics available to cycle through (first option is default) (OpticIDs)
	TArray<FName> CurrentAvailableOptics = { FName("NAME_None") };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentOpticIndex = 0;

	//current magnfication index?

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool isOn = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool isMagnified = false;
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
struct FHelicopterState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TArray<float> RotorRPMs; //first 1 should be main rotor

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float CurrentHoverVelocity = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float CurrentPitchSpeed = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float CurrentYawSpeed = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float CurrentRollSpeed = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float CurrentForwardMomentum = 0.0f;
};

USTRUCT(BlueprintType)
struct FJetState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float CurrentElevatorPitch = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float CurrentAileronRoll = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float CurrentRudderYaw = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float CurrentThrottle = 0.0f;
};

USTRUCT(BlueprintType)
struct FAircraftState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLandingGearIsDown = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FHelicopterState HelicopterState = FHelicopterState();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FJetState JetState = FJetState();
};

USTRUCT(BlueprintType)
struct FGenericVehicleState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	class UAudioComponent* EngineAudioComponent = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	class UAudioComponent* InteriorAudioComponent = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	class UAudioComponent* HornAudioComponent = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CurrentCamo = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool LoadoutApplied = false;			//<--- change to string, denote WHO's loadout it is? (might be good for comparing if different people hop into the main seat)
};

USTRUCT(BlueprintType)
struct FVehicleCurrentState
{
	GENERATED_BODY()
	//health?

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



