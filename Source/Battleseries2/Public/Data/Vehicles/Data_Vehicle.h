// Fill out your copyright notice in the Description page of Project Settings.
//Data_Vehicle.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "InputMappingContext.h"
#include "Sound/SoundCue.h"
#include "ChaosVehicleWheel.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Data/Vehicles/Data_Seat.h"
#include "Data/Vehicles/VehicleEnums.h"
#include "Data_Vehicle.generated.h"

USTRUCT(BlueprintType)
struct FGenericVehicleAudio
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	class USoundCue* EngineAudio = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	class USoundCue* EngineStartupAudio = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	class USoundCue* EngineShutdownAudio = nullptr;

	//interior audio?
	//wheel audio?
};

USTRUCT(BlueprintType)
struct FTurretData_PitchAndRotation
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FFloatRange TurretMinMax = FFloatRange(0.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float TurretSpeed = 0.0f;
};

USTRUCT(BlueprintType)
struct FTurretData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool TurretMeshIsSeparateWeaponMesh = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "TurretMeshIsSeparateWeaponMesh == false", EditConditionHides))
	FName TurretSocket = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FTurretData_PitchAndRotation TurretPitch = FTurretData_PitchAndRotation();
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FTurretData_PitchAndRotation TurretRotation = FTurretData_PitchAndRotation();
};

USTRUCT(BlueprintType)
struct FVehicleCamoData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<int32, UMaterialInstance*> MaterialElementIndexMap;		//maps the MI with the material slot on the mesh 
};

USTRUCT(BlueprintType)
struct FBaseWheelData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = Shape)
	TObjectPtr<class UStaticMesh> CollisionMesh;

	/** If left undefined then the bAffectedByEngine value is used, if defined then bAffectedByEngine is ignored and the differential setup on the vehicle defines which wheels get power from the engine */
	UPROPERTY(EditAnywhere, Category = Wheel)
	EAxleType AxleType;

	/**
	 * If BoneName is specified, offset the wheel from the bone's location.
	 * Otherwise this offsets the wheel from the vehicle's origin.
	 */
	UPROPERTY(EditAnywhere, Category = Wheel)
	FVector Offset;

	/** Radius of the wheel */
	UPROPERTY(EditAnywhere, Category = Wheel, meta = (ClampMin = "0.01", UIMin = "0.01"))
	float WheelRadius;

	/** Width of the wheel */
	UPROPERTY(EditAnywhere, Category = Wheel, meta = (ClampMin = "0.01", UIMin = "0.01"))
	float WheelWidth;

	/** Mass of the wheel Kg */
	UPROPERTY(EditAnywhere, Category = Wheel, meta = (ClampMin = "0.01", UIMin = "0.01"))
	float WheelMass;

	/** Tyre Cornering Ability */
	UPROPERTY(EditAnywhere, Category = Wheel)
	float CorneringStiffness;

	/** Friction Force Multiplier */
	UPROPERTY(EditAnywhere, Category = Wheel)
	float FrictionForceMultiplier;

	/** Wheel Lateral Skid Grip Loss, lower number less grip on skid */
	UPROPERTY(EditAnywhere, Category = Wheel, meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"))
	float SideSlipModifier;

	/** Wheel Longitudinal Slip Threshold */
	UPROPERTY(EditAnywhere, Category = Wheel, meta = (ClampMin = "0.0", UIMin = "0.0"))
	float SlipThreshold;

	/** Wheel Lateral Skid Threshold */
	UPROPERTY(EditAnywhere, Category = Wheel, meta = (ClampMin = "0.0", UIMin = "0.0"))
	float SkidThreshold;

	// steer angle in degrees for this wheel
	UPROPERTY(EditAnywhere, Category = WheelsSetup)
	float MaxSteerAngle;

	/** Whether steering should affect this wheel */
	UPROPERTY(EditAnywhere, Category = WheelsSetup)
	bool bAffectedBySteering;

	/** Whether brake should affect this wheel */
	UPROPERTY(EditAnywhere, Category = Wheel)
	bool bAffectedByBrake;

	/** Whether handbrake should affect this wheel */
	UPROPERTY(EditAnywhere, Category = Wheel)
	bool bAffectedByHandbrake;

	/** Whether engine should power this wheel */
	UPROPERTY(EditAnywhere, Category = Wheel)
	bool bAffectedByEngine;

	/** Advanced Braking System Enabled */
	UPROPERTY(EditAnywhere, Category = Wheel)
	bool bABSEnabled;

	/** Straight Line Traction Control Enabled */
	UPROPERTY(EditAnywhere, Category = Wheel)
	bool bTractionControlEnabled;

	/** Max Wheelspin rotation rad/sec */
	UPROPERTY(EditAnywhere, Category = Wheel)
	float MaxWheelspinRotation;

	/** Determines how the SetDriveTorque/SetBrakeTorque inputs are combined with the internal torques */
	UPROPERTY(EditAnywhere, Category = Wheel)
	ETorqueCombineMethod ExternalTorqueCombineMethod;

	UPROPERTY(EditAnywhere, Category = Setup)
	FRuntimeFloatCurve LateralSlipGraph;

	/** Local body direction in which where suspension forces are applied (typically along -Z-axis) */
	UPROPERTY(EditAnywhere, Category = Suspension)
	FVector SuspensionAxis;

	/** Vertical offset from where suspension forces are applied (along Z-axis) */
	UPROPERTY(EditAnywhere, Category = Suspension)
	FVector SuspensionForceOffset;

	/** How far the wheel can go above the resting position */
	UPROPERTY(EditAnywhere, Category = Suspension)
	float SuspensionMaxRaise;

	/** How far the wheel can drop below the resting position */
	UPROPERTY(EditAnywhere, Category = Suspension)
	float SuspensionMaxDrop;

	/** Suspension damping, larger value causes the suspension to come to rest faster [range 0 to 1] */
	UPROPERTY(EditAnywhere, Category = Suspension)
	float SuspensionDampingRatio;

	/**
	 *	Amount wheel load effects wheel friction.
		At 0 wheel friction is completely independent of the loading on the wheel (This is artificial as it always assumes even balance between all wheels)
		At 1 wheel friction is based on the force pressing wheel into the ground. This is more realistic.
		Lower value cures lift off over-steer, generally makes vehicle easier to handle under extreme motions.
	 */
	UPROPERTY(EditAnywhere, Category = Suspension, meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"))
	float WheelLoadRatio;

	/** Spring Force (N/m) */
	UPROPERTY(EditAnywhere, Category = Suspension)
	float SpringRate;

	/** Spring Preload (N/m) */
	UPROPERTY(EditAnywhere, Category = Suspension)
	float SpringPreload;

	/** Smooth suspension [0-off, 10-max] - Warning might cause momentary visual inter-penetration of the wheel against objects/terrain */
	UPROPERTY(EditAnywhere, Category = Suspension, meta = (ClampMin = "0.0", UIMin = "0", ClampMax = "10.0", UIMax = "10"))
	int SuspensionSmoothing;

	/** Anti-roll effect */
	UPROPERTY(EditAnywhere, Category = Suspension, meta = (ClampMin = "0.0", UIMin = "0", ClampMax = "1.0", UIMax = "1"))
	float RollbarScaling;

	/** Wheel suspension trace type, defaults to ray trace */
	UPROPERTY(EditAnywhere, Category = Suspension)
	ESweepShape SweepShape;

	/** Whether wheel suspension considers simple, complex */
	UPROPERTY(EditAnywhere, Category = Suspension)
	ESweepType SweepType;

	/** max brake torque for this wheel (Nm) */
	UPROPERTY(EditAnywhere, Category = Brakes)
	float MaxBrakeTorque;

	/**
	 *	Max handbrake brake torque for this wheel (Nm). A handbrake should have a stronger brake torque
	 *	than the brake. This will be ignored for wheels that are not affected by the handbrake.
	 */
	UPROPERTY(EditAnywhere, Category = Brakes)
	float MaxHandBrakeTorque;
};

USTRUCT(BlueprintType)
struct FGroundVehicleData
{
	GENERATED_BODY()

	//STEERING DATA
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool canIdleTurn = false;

	//THE FOLLOWING ARE CHAOSWHEELEDVEHICLEMOVEMENT PROPERTIES/DATA & SHOULD BE BAKED/COPIED ACCORDINGLY
	//Wheel Setup
	UPROPERTY(EditAnywhere)
	TArray<FBaseWheelData> BaseWheelData;
	UPROPERTY(EditAnywhere)
	TArray<FChaosWheelSetup> WheelData;
	//Engine Setup
	UPROPERTY(EditAnywhere)
	FVehicleEngineConfig EngineData = FVehicleEngineConfig();

	UPROPERTY(EditAnywhere)
	FVehicleDifferentialConfig DifferentialData = FVehicleDifferentialConfig();

	UPROPERTY(EditAnywhere)
	FVehicleTransmissionConfig TransmissionData = FVehicleTransmissionConfig();

	UPROPERTY(EditAnywhere)
	FVehicleSteeringConfig SteeringData = FVehicleSteeringConfig();

	//Vehicle Setup
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Mass = 0.0f;

	//Vehicle Input
	//Yaw
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Yaw_Input_Rise_Rate = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Yaw_Input_Fall_Rate = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector Center_Of_Mass_Override = FVector();

	//other
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bCanAmphibiousTravel = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	class USoundCue* HornAudio = nullptr;
	//wheel audio?
};

USTRUCT(BlueprintType)
struct FRotorData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float TargetRPM = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FFloatRange MinMaxAccelerationSpeed = FFloatRange(0.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float AccelerationCurve = 0.0f;
};

USTRUCT(BlueprintType)
struct FHelicopterData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRotorData RotorData = FRotorData();

	//PITCH DATA
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PitchSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxPitchSpeed = 0.0f;

	//YAW DATA
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float YawSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxYawSpeed = 0.0f;

	//ROLL DATA
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RollSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxRollSpeed = 0.0f;
};

//THE ONE THAT WE MAKE DATA TABLE OUT OF
USTRUCT(BlueprintType)
struct FVehicleData : public FTableRowBase			//<-- makes it accessible for data tables
{
	GENERATED_BODY()

	//generic stuff/things applied to all vehicles
	//THE FOLLOWING DATA SHOULD NOT BE RUNTIME DATA
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Vehicle_DisplayName = FText();

	//NOT VEHICLE TYPE, MOVEMENT TYPE (GROUNDVEHICLE, JET, HELICOPTER, BOAT)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	E_MovementType Movement_Type = E_MovementType::GroundVehicle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EVehicleType Vehicle_Type = EVehicleType::Tank;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> VehicleIcon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<USkeletalMesh> Vehicle_Mesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UPhysicsAsset> Preview_PhysicsAsset = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FTransform CustomizationPosition = FTransform();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftClassPtr<UAnimInstance> Anim_Class = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGenericVehicleAudio GenericVehicleAudio = FGenericVehicleAudio();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FSeatData> Seats;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, FVehicleCamoData> AvailableCamos;		//CamoID, MI for that vehicle

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FTurretData> Turrets;

	//GROUND VEHICLE SPECIFIC STUFF
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "Movement_Type == E_MovementType::GroundVehicle", EditConditionHides))
	FGroundVehicleData GroundVehicle_Data = FGroundVehicleData();

	//HELICOPTER SPECIFIC STUFF
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "Movement_Type == E_MovementType::Helicopter", EditConditionHides))
	FHelicopterData Helicopter_Data = FHelicopterData();

	// --- Static helper function to get row names by vehicle type ---
	static TArray<FName> GetRowNamesOfType(UDataTable* VehicleDataTable, EVehicleType TypeToFilter)
	{
		FString TypeName = StaticEnum<EVehicleType>()->GetNameStringByValue((int64)TypeToFilter);
		TArray<FName> AllVehicleIDsOfType;
		const TArray<FName>& AllVehicleIDs = VehicleDataTable->GetRowNames();
		for (FName VehicleID : AllVehicleIDs)
		{
			const FVehicleData* VehicleData = VehicleDataTable->FindRow<FVehicleData>(VehicleID, TEXT("Filter by Vehicle Type"));
			if (VehicleData->Vehicle_Type == TypeToFilter)
			{
				AllVehicleIDsOfType.Add(VehicleID); // <-- return the actual row name
			}
		}
		return AllVehicleIDsOfType;
	}
};