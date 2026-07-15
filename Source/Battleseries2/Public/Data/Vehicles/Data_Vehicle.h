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
	TObjectPtr<USoundCue> EngineAudio = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<USoundCue> EngineStartupAudio = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<USoundCue> EngineShutdownAudio = nullptr;

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
struct FIdleTurnData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool canIdleTurn = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float TargetTorqueScale = 0.0f;			// Power of the turn (increased since drag will fight it)

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float AngularDragScale = 0.0f;   // How hard the tank fights its own rotation speed
};

USTRUCT(BlueprintType)
struct FVehicleSetup
{
	//STRUCT THAT LIFTS/COPIES PROPERTIES FROM CHAOS "VEHICLE SETUP" section
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = VehicleSetup)
	bool bReverseAsBrake = false;

	/** If true, when reversing the throttle will behave like a brake while the vehicle moving in a backwards direction - requires bReverseAsBrake to be enabled for operation */
	UPROPERTY(EditAnywhere, Category = VehicleSetup, meta = (EditCondition = "bReverseAsBrake"))
	bool bThrottleAsBrake = false;

	/** Mass to set the vehicle chassis to. It's much easier to tweak vehicle settings when
	 * the mass doesn't change due to tweaks with the physics asset. [kg] */
	UPROPERTY(EditAnywhere, Category = VehicleSetup, meta = (ClampMin = "0.01", UIMin = "0.01"))
	float Mass = 1.0f;

	/**
	 * Enable to override the calculated COM position with your own fixed value - this prevents the vehicle handling changing when the asset changes
	 */
	UPROPERTY(EditAnywhere, Category = VehicleSetup)
	bool bEnableCenterOfMassOverride = false;

	/**
	 * The center of mass override value, this value overrides the calculated COM and the COM offset value in the mesh is also ignored.
	 */
	UPROPERTY(EditAnywhere, Category = VehicleSetup, meta = (EditCondition = "bEnableCenterOfMassOverride"))
	FVector CenterOfMassOverride = FVector();

	/** Chassis width used for drag force computation (cm)*/
	UPROPERTY(EditAnywhere, Category = VehicleSetup, meta = (ClampMin = "0.01", UIMin = "0.01"))
	float ChassisWidth = 0.0f;

	/** Chassis height used for drag force computation (cm)*/
	UPROPERTY(EditAnywhere, Category = VehicleSetup, meta = (ClampMin = "0.01", UIMin = "0.01"))
	float ChassisHeight = 0.0f;

	/** DragCoefficient of the vehicle chassis - force resisting forward motion at speed */
	UPROPERTY(EditAnywhere, Category = VehicleSetup)
	float DragCoefficient = 0.0f;

	/** DownforceCoefficient of the vehicle chassis - force pressing vehicle into ground at speed */
	UPROPERTY(EditAnywhere, Category = VehicleSetup)
	float DownforceCoefficient = 0.0f;

	// Drag area in Meters^2
	UPROPERTY(transient)
	float DragArea = 0.0f;

	// Debug drag magnitude last applied
	UPROPERTY(transient)
	float DebugDragMagnitude = 0.0f;

	/** Scales the vehicle's inertia in each direction (forward, right, up) */
	UPROPERTY(EditAnywhere, Category = VehicleSetup, AdvancedDisplay)
	FVector InertiaTensorScale = FVector();

	/** Option to apply some aggressive sleep logic, larger number is more agressive, 0 disables */
	UPROPERTY(EditAnywhere, Category = VehicleSetup)
	float SleepThreshold = 0.0f;

	/** Option to apply some aggressive sleep logic if slopes up Z is less than this value, i.e value = Cos(SlopeAngle) so 0.866 will sleep up to 30 degree slopes */
	UPROPERTY(EditAnywhere, Category = VehicleSetup, meta = (ClampMin = "0.01", UIMin = "0.01", ClampMax = "1.0", UIMax = "1.0"))
	float SleepSlopeLimit = 0.0f;
};

USTRUCT(BlueprintType)
struct FGroundVehicleData
{
	GENERATED_BODY()

	//STEERING DATA
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FIdleTurnData IdleTurnData = FIdleTurnData();

	//THE FOLLOWING ARE CHAOSWHEELEDVEHICLEMOVEMENT PROPERTIES/DATA & SHOULD BE BAKED/COPIED ACCORDINGLY

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVehicleSetup VehicleSetup = FVehicleSetup();

	//Vehicle Input
	//Yaw
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Yaw_Input_Rise_Rate = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Yaw_Input_Fall_Rate = 0.0f;

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
struct FAircraftData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TArray<FChaosWheelSetup> LandingGear;
};

USTRUCT(BlueprintType)
struct FHelicopterData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRotorData RotorData = FRotorData();

	//THROTTLE/HOVER/THRUST DATA
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight|Momentum", meta = (ToolTip = "How fast Forward Momentum builds per second"))
	float Acceleration = 4.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight|Momentum", meta = (ToolTip = "Cap on accumulated forward speed"))
	float MaxMomentum = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight|Momentum", meta = (ToolTip = "Minimum accumulated forward speed (allows reverse)"))
	float MinMomentum = -50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight|Throttle", meta = (ToolTip = "Vertical force needed to hover (neutral throttle)"))
	float HoverPower = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight|Throttle", meta = (ToolTip = "Max vertical climb force"))
	float MaxThrust = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight|Throttle", meta = (ToolTip = "Downward pull when below hover power"))
	float Gravity = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight|Rotation")
	float MaxPitchSpeed = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight|Rotation")
	float MaxYawSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight|Rotation")
	float MaxRollSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxVelocity = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight|Curves", meta = (ToolTip= "Maps input to pitch response curve"))
	TObjectPtr<UCurveFloat> PitchCurve = nullptr;
};

USTRUCT(BlueprintType)
struct FThrottleFlightModel
{
	GENERATED_BODY()

	// --- Actuator / Spool Settings ---

	// How fast the engine responds to input (Spool Speed)
	UPROPERTY(EditAnywhere, Category = "Actuator", meta = (ClampMin = "0.1"))
	float ThrottleSpeed = 3.0f;

	// --- Power Settings ---

	// The "Power" of the engine. 
	// Kinematic: Max speed in cm/s | Dynamic: Thrust force/accel
	UPROPERTY(EditAnywhere, Category = "Power")
	float ThrustStrength = 5000.0f;

	// --- Landing & Takeoff ---

	// Speed (KMH) required before the nose can lift
	UPROPERTY(EditAnywhere, Category = "Limits")
	float TakeoffVelocity = 180.0f;

	// Power multiplier when landing gear is out (0.5 = 50% power)
	UPROPERTY(EditAnywhere, Category = "Limits", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float GearDownSpeedScalar = 0.5f;
};

USTRUCT(BlueprintType)
struct FPitchFlightModel
{
	GENERATED_BODY()

	// --- Actuator / Response Settings ---

	// How fast the elevator flaps move to target
	UPROPERTY(EditAnywhere, Category = "Actuator")
	float PitchSpeed = 5.0f;

	// Maximum deflection of the elevators (0.0 to 1.0)
	UPROPERTY(EditAnywhere, Category = "Actuator")
	float InputLimit = 1.0f;

	// --- Power Settings ---

	// The "Agility" of the pitch.
	// Kinematic: Degrees/sec | Dynamic: Torque force
	UPROPERTY(EditAnywhere, Category = "Power")
	float PitchStrength = 90.0f;

	// --- Sensitivity ---

	// X: Speed in KMH, Y: Multiplier (0.0 - 1.0)
	//kinematic: FinalDegreesPerSecond = PitchStrength * SensitivityCurve
	//dynamic: AppliedTorque = TorqueStrength * SensitivityCurve
	//chaos input: ChaosPitchInput = RawInput * SensitivityCurve
	UPROPERTY(EditAnywhere, Category = "Sensitivity")
	TObjectPtr<UCurveFloat> PitchSensitivityCurve;
};

USTRUCT(BlueprintType)
struct FRollFlightModel
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RollSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RollStrength = 0.0f;
};

USTRUCT(BlueprintType)
struct FYawFlightModel
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float YawSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float YawStrength = 0.0f;
};


USTRUCT(BlueprintType)
struct FFlightModel_Chaos
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = AerofoilSetup)
	TArray<FVehicleAerofoilConfig> Aerofoils;

	/** Optional thruster setup, use one or more as your main engine or as supplementary booster */
	UPROPERTY(EditAnywhere, Category = ThrusterSetup)
	TArray<FVehicleThrustConfig> Thrusters;

	/** Arcade style direct control of vehicle rotation via torque force */
	UPROPERTY(EditAnywhere, Category = ArcadeControl)
	FVehicleTorqueControlConfig TorqueControl = FVehicleTorqueControlConfig();
	/** Arcade style direct control of vehicle rotation via torque force */
	UPROPERTY(EditAnywhere, Category = ArcadeControl)
	FVehicleTargetRotationControlConfig TargetRotationControl = FVehicleTargetRotationControlConfig();

	/** Arcade style control of vehicle */
	UPROPERTY(EditAnywhere, Category = ArcadeControl)
	FVehicleStabilizeControlConfig StabilizeControl = FVehicleStabilizeControlConfig();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVehicleSetup VehicleSetup = FVehicleSetup();
};

USTRUCT(BlueprintType)
struct FJetFlightModel
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Throttle")
	FThrottleFlightModel Throttle = FThrottleFlightModel();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pitch")
	FPitchFlightModel Pitch = FPitchFlightModel();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Roll")
	FRollFlightModel Roll = FRollFlightModel();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Yaw")
	FYawFlightModel Yaw = FYawFlightModel();
};

USTRUCT(BlueprintType)
struct FJetData
{
	GENERATED_BODY()

	// The "Master Switch" for the whole aircraft's logic
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "General")
	EFlightModelType FlightModelType = EFlightModelType::Dynamic;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight Model")
	FJetFlightModel FlightModel = FJetFlightModel();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FFlightModel_Chaos ChaosFlightModel = FFlightModel_Chaos();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVehicleSetup VehicleSetup = FVehicleSetup();
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "Movement_Type == E_MovementType::Jet", EditConditionHides))
	FJetData Jet_Data = FJetData();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "Movement_Type == E_MovementType::Helicopter || Movement_Type == E_MovementType::Jet", EditConditionHides))
	FAircraftData Aircraft_Data = FAircraftData();

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