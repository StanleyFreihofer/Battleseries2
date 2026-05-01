// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
// Interfaces MUST be included for inheritance
#include "Utilities/I_VehicleDataAccessor.h"
#include "Utilities/I_Anims.h"
#include "Core/Weapons/I_LockOnTarget.h"
// Structs used by VALUE must be included
#include "Data/Vehicles/Data_Vehicle.h" 
#include "Data/Core/CoreTypes.h"
#include "Data/Runtime/VehicleTypes.h"
// UBT/UHT requirements
#include "Engine/DataTable.h" 
#include "Vehicle_Base.generated.h"

class UChaosWheeledVehicleMovementComponent;
class UWidgetComponent;
class UCameraComponent;
class USpawnComponent;
class UVehicleWeaponLogicComponent;
class USkeletalMeshComponent;
class USkeletalMesh;
class USoundBase;
class ACharacter_Base;
class USaveSubsystem;
class UHUDSubsystem;
class UDataManagerSubsystem;
class UAnimInstance;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSeatsInitialized);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMeshReady);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVehicleYawUpdate, float, Yaw);		//broadcast when vehicle is turned/steered/yawed
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVehicleSpeedUpdate, float, Speed);		//broadcast on when throttle input




USTRUCT(BlueprintType)
struct FVehicleStartingData
{
	GENERATED_BODY()

	//what this vehicle is on start
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instance")
	FName VehicleID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instance")
	bool PreviewVehicle = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instance", meta = (tooltip = "use to permanently occupy certain seats for lifetime of vehicle"))			
	TArray<int32> OccupiedSeats;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instance")
	FPlayerLoadoutConfig_Vehicle StartingVehicleLoadout = FPlayerLoadoutConfig_Vehicle();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instance", meta = (tooltip = "if on, this will prevent any player/custom loadouts from being applied to the vehicle on enter of it for its lifetime"))
	bool LockLoadout = false;													
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instance")		
	float StartingHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instance")			
	float KillSwitchTimer = -1.0f;
};

UCLASS()
class BATTLESERIES2_API AVehicle_Base : public APawn, public IVehicleDataAccessor, public ILockOnTarget, public IAnims
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AVehicle_Base();

	//COMPONENTS
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	USkeletalMeshComponent* VehicleMeshComponent = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UWidgetComponent* InteractionWidgetComponent = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UChaosWheeledVehicleMovementComponent* ChaosVehicleMovement;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UVehicleWeaponLogicComponent* VehicleWeaponLogicComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	USpawnComponent* SpawnComponent;



	//VARIABLES
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true"), Category = "__Instance")
	FVehicleStartingData VehicleStartingData = FVehicleStartingData();
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "__Runtime")
	FVehicleCurrentState VehicleCurrentState = FVehicleCurrentState();

	//Static Data
	const FVehicleData* VehicleData;				//not a pointer

	UPROPERTY(BlueprintAssignable, Category = "Event Dispatchers")
	FOnSeatsInitialized OnSeatsInitialized;
	UPROPERTY(BlueprintAssignable, Category = "Event Dispatchers")
	FOnMeshReady OnMeshReady;

	UPROPERTY(BlueprintAssignable, Category = "Event Dispatchers")
	FOnVehicleYawUpdate OnVehicleYawUpdate;
	UPROPERTY(BlueprintAssignable, Category = "Event Dispatchers")
	FOnVehicleSpeedUpdate OnVehicleSpeedUpdate;





	//FUNCTIONS

	//INITALIZATION FUNCTIONS
	//Ground Vehicle init functions
	UFUNCTION(BlueprintCallable)
	void Init_Wheels();
	UFUNCTION(BlueprintCallable)
	void HandleChaosMovement(bool turnon);
	UFUNCTION(BlueprintCallable)
	void Init_EngineAudio();
	UFUNCTION(BlueprintCallable)
	void Init_GroundVehicle();
	UFUNCTION(BlueprintCallable)
	void Init_Helicopter();
	UFUNCTION(BlueprintCallable)
	void Init_DefaultSeatRemoteCamera(int32 SeatIndex);
	UFUNCTION(BlueprintCallable)
	void Init_SeatHUDComp(int32& SeatIndex);
	UFUNCTION(BlueprintCallable)
	void Init_Seats();
	UFUNCTION(BlueprintCallable)
	void Init_VehicleMesh(USkeletalMesh* LoadedSkeletalMesh);
	UFUNCTION(BlueprintCallable)
	void Init_VehicleAnim(TSubclassOf<UAnimInstance> Anim_Class);
	UFUNCTION(BlueprintCallable)
	void Init_VehicleData();
	UFUNCTION(BlueprintCallable)
	void Init_DetermineVehicleBuildBehavior();
	UFUNCTION(BlueprintCallable)
	void Init_Vehicle();
	UFUNCTION(BlueprintCallable)
	void Init_Horn(USoundBase* HornAudio);

	UFUNCTION(BlueprintCallable)
	void PlayHorn();
	UFUNCTION(BlueprintCallable)
	void StopHorn();

	UFUNCTION(BlueprintCallable)
	void Init_Vehicle_Preview();

	UFUNCTION(BlueprintCallable)
	void Init_VehicleMesh_Preview(USkeletalMesh* LoadedSkeletalMesh);


	//RUNTIME FUNCTIONS
	//Seats RUNTIME
	UFUNCTION(BlueprintCallable, Category = "Seats", meta = (DisplayName = "Cycle Through Seats", ReturnDisplayName = "Found Seat"))
	bool CycleThroughSeats(ACharacter_Base* Character);

	UFUNCTION(BlueprintCallable)
	void HandleViewMethod(ACharacter_Base* Character, const FSeatData& SeatData);

	UFUNCTION(BlueprintCallable)
	void HandleViewMethod_Default(ACharacter_Base* Character, const FSeatData& SeatData);

	UFUNCTION(BlueprintCallable)
	void HandleSeatOccupationStatus(bool Occupy, int32 SeatIndex);

	UFUNCTION(BlueprintCallable)
	void ApplyLoadoutToSeat(int32 SeatIndex);
	UFUNCTION(BlueprintCallable)
	void ApplyOpticToSeat(int32 SeatIndex);
	UFUNCTION(BlueprintCallable)
	void ApplyLoadoutToVehicle();
	UFUNCTION(BlueprintCallable)
	void ApplyCamoToVehicle(FName CamoID);
	UFUNCTION()
	void ApplyCamoToAttachment(TWeakObjectPtr<UMeshComponent> Mesh, FName AttachmentID, FName CamoID);

	UFUNCTION(BlueprintCallable)
	void ClearLoadoutFromSeat(int32 SeatIndex);
	UFUNCTION(BlueprintCallable)
	void ClearEntireLoadoutFromVehicle();

	UFUNCTION(BlueprintCallable)
	void DropSeat(ACharacter_Base* Character, int32& SeatIndex);

	UFUNCTION(BlueprintCallable)
	void SetupNewSeat(ACharacter_Base* Character);
	UFUNCTION(BlueprintCallable)
	void SetupDriver(ACharacter_Base* Character);
	UFUNCTION()
	void DropDriver();
	UFUNCTION(BlueprintCallable)
	void SetupGunner(ACharacter_Base* Character);
	UFUNCTION()
	void DropGunner(TWeakObjectPtr<ACharacter_Base> Character, int32& SeatIndex);

	UFUNCTION(BlueprintCallable)
	void AttemptEnterVehicle(ACharacter_Base* Character);

	UFUNCTION(BlueprintCallable)
	void ChangeSeat(ACharacter_Base* Character);

	UFUNCTION(BlueprintCallable)
	void UpdateSeatActiveCamera(int32 SeatIndex, UCameraComponent* NewActiveCamera);
	UFUNCTION(BlueprintCallable)
	void UpdateRemoteCamPP(int32 SeatIndex, FPostProcessSettings PPSettings, float BlendWeight);
	UFUNCTION(BlueprintCallable)
	void UpdateEngineAudio();
	UFUNCTION(BlueprintCallable)
	void UpdateRotorRPM();

	//Vehicle RUNTIME
	
	//INPUTS

	//Movement, Generic
	UFUNCTION(BlueprintCallable)
	void Input_HandleThrottle(float ThrottleValue);		//called from seat
	UFUNCTION(BlueprintCallable)
	void Input_ReleaseThrottle();
	//Movement, Ground Vehicle
	UFUNCTION(BlueprintCallable)
	void UpdateThrottle_GV(float InputValue);	
	UFUNCTION(BlueprintCallable)
	void Input_UpdateSteering_GV(float SteeringValue, int32 SeatIndex);

	UFUNCTION(BlueprintCallable)
	void UpdateThrottle_Heli(float InputValue);
	UFUNCTION(BlueprintCallable)
	void UpdatePitch_Heli(float InputValue);
	UFUNCTION(BlueprintCallable)
	void UpdateYaw_Heli(float InputValue);
	UFUNCTION(BlueprintCallable)
	void UpdateRoll_Heli(float InputValue);


	UFUNCTION(BlueprintCallable)
	void ToggleOptic(int32 SeatIndex);
	UFUNCTION(BlueprintCallable)
	void TurnOnPPOptic(int32 SeatIndex);
	UFUNCTION(BlueprintCallable)
	void TurnOffPPOptic(int32 SeatIndex, int32 PreviousOpticIndex);
	UFUNCTION(BlueprintCallable)
	void ToggleMagnificationOptic(int32 SeatIndex, float ZoomMagnification);

	//Applying
	UFUNCTION(BlueprintCallable)
	UCameraComponent* SpawnAndAttachCamera(FName SocketToAttach, USkeletalMeshComponent* MeshToAttachTo);

	UFUNCTION(BlueprintCallable)
	void SetVehicleAndInit(FVehicleStartingData InputVehicleStartingData);		//used to hook in during runtime i guess? (also calls init/kicks off vehicle setup)

	UFUNCTION(BlueprintCallable)
	void UpdateSeatList_AllOccupants();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	int32 GetControlledTurret(int32 SeatIndex);
	UFUNCTION(BlueprintCallable, BlueprintPure)
	UCameraComponent* GetRemoteActiveCam(int32 SeatIndex);

	UFUNCTION()
	TWeakObjectPtr<UHUDSubsystem> GetHUDSystem();
	UFUNCTION()
	UDataManagerSubsystem* GetDataManager();

	//Interfaces
	virtual USkeletalMeshComponent* GetVehicleMesh() const override;
	virtual FName GetVehicleID() const override;
	virtual const FVehicleData& GetVehicleData() const override;
	virtual const FVehicleCurrentState& GetVehicleState() const override;
	virtual AVehicle_Base& GetVehicle() override;

	virtual bool GetIfCanLockOn_Implementation(const TArray<ETargetingCategory>& TargetingCategories, EHomingCapability HomingCapability) override;

	//Getters

	//BP Wrappers
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FVehicleData GetVehicleData_BP() { return GetVehicleData(); }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FVehicleCurrentState GetVehicleState_BP() { return GetVehicleState(); }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;



public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	//UDataManagerSubsystem* DataManager;
	USaveSubsystem* SaveSubsystem;
	FTimerHandle TimerHandle_AudioUpdate_Engine;
	FTimerHandle SpeedTimer;
	FTimerHandle RotorUpdateTimer;
};
