// Fill out your copyright notice in the Description page of Project Settings.

#include "Vehicle_Base.h"
#include "GameFramework/PlayerController.h"			//need because we currently call setviewtarget
#include "Camera/CameraActor.h"
#include "Data/Vehicles/Data_Vehicle.h"				//need to access members
#include "Data/Vehicles/Data_Seat.h"				//need to access members
#include "Data/Data_VehicleAttachments.h"
#include "Data/Data_Optics.h"
#include "Data/Vehicles/VehicleDefaults.h"
#include "Core/Weapons/Projectiles/Projectile_Base.h"
#include "Core/Weapons/VehicleWeaponLogicComponent.h"
#include "Core/Vehicles/ChaosWheel_Base.h"
#include "Core/PlayerController_Base.h"
#include "Core/UI/VehicleHUDs/UW_HUD_Vehicle_Base.h"
#include "Core/SpawnComponent.h"
#include "Save/PlayerSave_Loadout.h"
#include "Save/SaveSubsystem.h"
#include "Utilities/GameInstance_Base.h"
#include "Utilities/DataManagerSubsystem.h"
#include "Utilities/ProjectilePoolSubsystem.h"
#include "Utilities/BS2FunctionLibrary.h"
#include "Utilities/HUDSubsystem.h"
#include "Utilities/BS2FunctionLibrary.h"
#include "Utilities/I_Anims.h"
#include "InputAction.h"
#include "Character_Base.h"							//need to access LSI, CSI, NSI... and probably other things
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Components/AudioComponent.h"
#include "ChaosVehicleMovementComponent.h"

// Sets default values
AVehicle_Base::AVehicle_Base()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	VehicleMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("VehicleMeshComponent"));
	SetRootComponent(VehicleMeshComponent); 
	ChaosVehicleMovement = CreateDefaultSubobject<UChaosWheeledVehicleMovementComponent>(TEXT("ChaosWheeledVehicleMovementComponent"));
	ChaosVehicleMovement->bAutoRegister = false;			
	ChaosVehicleMovement->bAutoActivate = false;	
	ChaosVehicleMovement->SetUpdatedComponent(VehicleMeshComponent);
	InteractionWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("Interaction Widget"));
	VehicleWeaponLogicComponent = CreateDefaultSubobject<UVehicleWeaponLogicComponent>(TEXT("Vehicle Weapon Logic Component"));
	SpawnComponent = CreateDefaultSubobject<USpawnComponent>(TEXT("Spawn Component"));
	SpawnComponent->SetupAttachment(GetRootComponent());
	InteractionWidgetComponent->SetupAttachment(VehicleMeshComponent, "InteractIcon");
}

// Called when the game starts or when spawned
void AVehicle_Base::BeginPlay()
{
	Super::BeginPlay();

	check(!VehicleStartingData.VehicleID.IsNone());			//enter the ID dummy
	if (UBS2FunctionLibrary::GetDataSubsystem(this)->IsDataReady())
	{
		Init_VehicleData();
	}
	else
	{
		UBS2FunctionLibrary::GetDataSubsystem(this)->OnDataReady.AddDynamic(this, &AVehicle_Base::Init_VehicleData);
	}
}

// Called every frame
void AVehicle_Base::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AVehicle_Base::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

#pragma region Initalization/Factory

void AVehicle_Base::Init_Wheels(const FWheelSetup& WheelSetup)
{
	ChaosVehicleMovement->bSuspensionEnabled = WheelSetup.SuspensionEnabled;
	ChaosVehicleMovement->EnableWheelFriction(WheelSetup.WheelFrictionEnabled);
	ChaosVehicleMovement->bLegacyWheelFrictionPosition = WheelSetup.LegacyWheelFrictionPosition;
	
	ChaosVehicleMovement->WheelSetups.Empty();
	ChaosVehicleMovement->Wheels.Empty();
	ChaosVehicleMovement->WheelSetups.SetNum(WheelSetup.WheelData.Num());
	for (int32 i = 0; i < WheelSetup.WheelData.Num(); i++)
	{
		const FChaosWheelSetup& SourceData = WheelSetup.WheelData[i];				//the data we are pulling from
		FChaosWheelSetup& Setup = ChaosVehicleMovement->WheelSetups[i];				//the properties we are applying to
		Setup.WheelClass = SourceData.WheelClass;
		Setup.BoneName = SourceData.BoneName;										//set bone name
		Setup.AdditionalOffset = SourceData.AdditionalOffset;						//set additional offset
	}
}

void AVehicle_Base::Init_GroundVehicle()
{
	if (!ChaosVehicleMovement->IsRegistered())
	{
		ChaosVehicleMovement->RegisterComponent();
	}
	VehicleMeshComponent->RecreatePhysicsState();
	//ChaosVehicleMovement->UnregisterComponent();
	//ChaosVehicleMovement->SetUpdatedComponent(VehicleMeshComponent);
	Init_Wheels(VehicleData->GroundVehicle_Data.WheelSetup);

	//Mechanical Setup
	ChaosVehicleMovement->EnableMechanicalSim(true);	//true by default, not in DTs
	ChaosVehicleMovement->EngineSetup = VehicleData->GroundVehicle_Data.EngineData;
	ChaosVehicleMovement->DifferentialSetup = VehicleData->GroundVehicle_Data.DifferentialData;
	ChaosVehicleMovement->TransmissionSetup = VehicleData->GroundVehicle_Data.TransmissionData;
	ChaosVehicleMovement->SteeringSetup = VehicleData->GroundVehicle_Data.SteeringData;
	
	//Vehicle Setup
	Init_Chaos_VehicleSetup(VehicleData->GroundVehicle_Data.VehicleSetup);

	//Vehicle Input
	//Yaw Input Rate
	ChaosVehicleMovement->YawInputRate.RiseRate = VehicleData->GroundVehicle_Data.Yaw_Input_Rise_Rate;
	ChaosVehicleMovement->YawInputRate.FallRate = VehicleData->GroundVehicle_Data.Yaw_Input_Fall_Rate;

	//allows the vehicle/chaos vehicle to actual simulate physics properly
	HandleChaosMovement(true);
	ChaosVehicleMovement->ResetVehicle();
	//ChaosVehicleMovement->RecreatePhysicsState();;
	VehicleMeshComponent->InitAnim(true);
}

void AVehicle_Base::HandleChaosMovement(bool turnon)
{
	if (turnon)
	{
		ChaosVehicleMovement->Activate();
		ChaosVehicleMovement->SetComponentTickEnabled(true);
	}
	else
	{
		ChaosVehicleMovement->Deactivate();
		ChaosVehicleMovement->SetComponentTickEnabled(false);
	}
}

void AVehicle_Base::Init_Chaos_VehicleSetup(const FVehicleSetup& VehicleSetup)
{
	//inputs custom made data struct of chaos property values into actual chaos properties under "Vehicle Setup" tab
	ChaosVehicleMovement->bReverseAsBrake = VehicleSetup.bReverseAsBrake;
	ChaosVehicleMovement->bThrottleAsBrake = VehicleSetup.bThrottleAsBrake;
	ChaosVehicleMovement->Mass = VehicleSetup.Mass;
	ChaosVehicleMovement->bEnableCenterOfMassOverride = VehicleSetup.bEnableCenterOfMassOverride;
	ChaosVehicleMovement->CenterOfMassOverride = VehicleSetup.CenterOfMassOverride;
	ChaosVehicleMovement->ChassisWidth = VehicleSetup.ChassisWidth;
	ChaosVehicleMovement->ChassisHeight = VehicleSetup.ChassisHeight;
	ChaosVehicleMovement->DragCoefficient = VehicleSetup.DragCoefficient;
	ChaosVehicleMovement->DownforceCoefficient = VehicleSetup.DownforceCoefficient;
	ChaosVehicleMovement->InertiaTensorScale = VehicleSetup.InertiaTensorScale;
	ChaosVehicleMovement->SleepThreshold = VehicleSetup.SleepThreshold;
	ChaosVehicleMovement->SleepSlopeLimit = VehicleSetup.SleepSlopeLimit;
}

void AVehicle_Base::Init_EngineAudio()
{
	VehicleCurrentState.GenericVehicleState.EngineAudioComponent = NewObject<UAudioComponent>(this, UAudioComponent::StaticClass());
	VehicleCurrentState.GenericVehicleState.EngineAudioComponent->RegisterComponent();
	VehicleCurrentState.GenericVehicleState.EngineAudioComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	VehicleCurrentState.GenericVehicleState.EngineAudioComponent->SetActive(true);
}

void AVehicle_Base::Init_Helicopter()
{
	VehicleCurrentState.AircraftState.HelicopterState.RotorRPMs.Add(0.0f);

	VehicleCurrentState.GenericVehicleState.InteriorAudioComponent = NewObject<UAudioComponent>(this, UAudioComponent::StaticClass());
	VehicleCurrentState.GenericVehicleState.InteriorAudioComponent->RegisterComponent();
	VehicleCurrentState.GenericVehicleState.InteriorAudioComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	VehicleCurrentState.GenericVehicleState.InteriorAudioComponent->SetActive(true);
}

void AVehicle_Base::Init_Jet_Chaos()
{
	Init_Wheels(VehicleData->Aircraft_Data.LandingGear);
	HandleChaosMovement(true);
	ChaosVehicleMovement->SetUpdatedComponent(VehicleMeshComponent);
	ChaosVehicleMovement->Aerofoils = VehicleData->Jet_Data.ChaosFlightModelData.Aerofoils;
	ChaosVehicleMovement->Thrusters = VehicleData->Jet_Data.ChaosFlightModelData.Thrusters;
	ChaosVehicleMovement->TorqueControl = VehicleData->Jet_Data.ChaosFlightModelData.TorqueControl;
	ChaosVehicleMovement->TargetRotationControl = VehicleData->Jet_Data.ChaosFlightModelData.TargetRotationControl;
	ChaosVehicleMovement->StabilizeControl = VehicleData->Jet_Data.ChaosFlightModelData.StabilizeControl;
	ChaosVehicleMovement->EnableMechanicalSim(true);

	//vehicle input
	ChaosVehicleMovement->ThrottleInputRate = VehicleData->Jet_Data.ChaosFlightModelData.VehicleInput.ThrottleInputRate;			
	ChaosVehicleMovement->BrakeInputRate = VehicleData->Jet_Data.ChaosFlightModelData.VehicleInput.BrakeInputRate;
	ChaosVehicleMovement->SteeringInputRate = VehicleData->Jet_Data.ChaosFlightModelData.VehicleInput.SteeringInputRate;
	ChaosVehicleMovement->HandbrakeInputRate = VehicleData->Jet_Data.ChaosFlightModelData.VehicleInput.HandbrakeInputRate;
	ChaosVehicleMovement->PitchInputRate = VehicleData->Jet_Data.ChaosFlightModelData.VehicleInput.PitchInputRate;
	ChaosVehicleMovement->RollInputRate = VehicleData->Jet_Data.ChaosFlightModelData.VehicleInput.RollInputRate;
	ChaosVehicleMovement->YawInputRate = VehicleData->Jet_Data.ChaosFlightModelData.VehicleInput.YawInputRate;

	Init_Chaos_VehicleSetup(VehicleData->Jet_Data.ChaosFlightModelData.VehicleSetup);
	ChaosVehicleMovement->RegisterComponent();
	ChaosVehicleMovement->RecreatePhysicsState();;
	VehicleMeshComponent->InitAnim(true);
}

void AVehicle_Base::Init_Optic(int32 SeatIndex)
{
	if (VehicleStartingData.StartingVehicleLoadout.SeatLoadout.Find(SeatIndex) && !VehicleStartingData.StartingVehicleLoadout.SeatLoadout.Find(SeatIndex)->Optic.IsNone())
	{
		VehicleCurrentState.SeatStates[SeatIndex].OpticState.CurrentAvailableOptics.Init(VehicleStartingData.StartingVehicleLoadout.SeatLoadout.Find(SeatIndex)->Optic, 1);
	}
	else
	{
		VehicleCurrentState.SeatStates[SeatIndex].OpticState.CurrentAvailableOptics.Init(VehicleData->Seats[SeatIndex].DefaultOptic, 1);
	}
}

#pragma region SeatInitialization

void AVehicle_Base::Init_DefaultSeatRemoteCamera(int32 SeatIndex)
{
	//init default seat camera
	//every seat that is remote should do this
	E_ViewMethod ViewMethod = VehicleData->Seats[SeatIndex].ViewMethod;
	FString CameraSocketString = FString::Printf(TEXT("SC_%02d"), SeatIndex);		//SeatCam_SeatIndex		[SC_00]
	FName CameraSocketName = FName(*CameraSocketString);
	TObjectPtr<UCameraComponent> NewCamera;
	switch (ViewMethod)
	{
		case E_ViewMethod::Remote:
			NewCamera = UBS2FunctionLibrary::CreateAndAttachCamera(this, VehicleMeshComponent, CameraSocketName);
			VehicleCurrentState.SeatStates[SeatIndex].DefaultCamera = NewCamera;
			UpdateSeatActiveCamera(SeatIndex, NewCamera);

			UpdateRemoteActiveCamPP(SeatIndex, UBS2FunctionLibrary::GetDataSubsystem(this)->GetOpticDataRow(VehicleCurrentState.SeatStates[SeatIndex].OpticState.CurrentAvailableOptics[VehicleCurrentState.SeatStates[SeatIndex].OpticState.CurrentOpticIndex])->OpticPPSettings, 1.0f, GetRemoteActiveCam(SeatIndex));
			break;
	}
}

void AVehicle_Base::Init_SeatHUDComp(int32& SeatIndex)
{
	TObjectPtr<UWidgetComponent> CockpitHUDComponent = NewObject<UWidgetComponent>(this);
	CockpitHUDComponent->RegisterComponent();
	CockpitHUDComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	CockpitHUDComponent->SetRelativeTransform(VehicleData->Seats[SeatIndex].DefaultCharacterContext.SeatHUDTransform);
	CockpitHUDComponent->SetWidgetClass(VehicleData->Seats[SeatIndex].DefaultCharacterContext.SeatHUD);
	CockpitHUDComponent->SetDrawSize(VehicleData->Seats[SeatIndex].DefaultCharacterContext.SeatHUDDrawSize);
	CockpitHUDComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CockpitHUDComponent->SetPivot(FVector2D(0.5f, 0.5f));
	CockpitHUDComponent->SetOwnerNoSee(false);
	VehicleCurrentState.SeatStates[SeatIndex].SeatHUDComponent = CockpitHUDComponent;
}

void AVehicle_Base::Init_Seats()
{
	VehicleCurrentState.SeatStates.SetNum(VehicleData->Seats.Num());
	for (int32 SI = 0; SI < VehicleData->Seats.Num(); ++SI)
	{
		const FSeatData& SeatInfo = VehicleData->Seats[SI];

		Init_Optic(SI);
		Init_DefaultSeatRemoteCamera(SI);	

		if (SeatInfo.DefaultCharacterContext.SeatHUD)
		{
			Init_SeatHUDComp(SI);
		}

		for (int32 i = 0; i < VehicleStartingData.OccupiedSeats.Num(); ++i)
		{
			if (VehicleStartingData.OccupiedSeats[i] == SI)
			{
				HandleSeatOccupationStatus(true, SI);
				//VehicleCurrentState.SeatStates[SI].isOccupied = true;
				break;
			}
		}
	}

	OnSeatsInitialized.Broadcast();		//here so we can guarantee ALL NEW SEATS ARE SETUP (race conditions suck)
	UE_LOG(LogTemp, Error, TEXT("[Vehicle_Base::Init_Seats] STEP 2B FINISHED (end of Init_Seats). old seats destroyed. new seats spawned in, VehicleID = %s"), *VehicleStartingData.VehicleID.ToString());
}

#pragma endregion

void AVehicle_Base::Init_VehicleMesh(USkeletalMesh* LoadedSkeletalMesh)
{
	//assumes the mesh loaded correctly/is valid
	//assumes there is physics asset tied to skelmeh by defaults
	VehicleMeshComponent->SetSkeletalMesh(LoadedSkeletalMesh);
	VehicleMeshComponent->SetSimulatePhysics(true);
	VehicleMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	VehicleMeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_Vehicle);
	VehicleMeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
}

void AVehicle_Base::Init_VehicleMesh_Preview(USkeletalMesh* LoadedSkeletalMesh)
{
	VehicleMeshComponent->SetSkeletalMesh(LoadedSkeletalMesh);

	if (!VehicleData->Preview_PhysicsAsset.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("Physics Asset is valid"));
		UPhysicsAsset* LoadedAsset = VehicleData->Preview_PhysicsAsset.LoadSynchronous();
		VehicleMeshComponent->SetPhysicsAsset(LoadedAsset, true);
	}
	VehicleMeshComponent->SetSimulatePhysics(true);
	VehicleMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	VehicleMeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_Vehicle);
	VehicleMeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	OnMeshReady.Broadcast();
}

void AVehicle_Base::Init_VehicleAnim(TSubclassOf<UAnimInstance> Anim_Class)
{
	//assumes mesh and anim class are valid
	VehicleMeshComponent->SetAnimInstanceClass(Anim_Class);
}

void AVehicle_Base::Init_VehicleData()			//(load vehicle data)
{
	//1. load base vehicle data row
	VehicleData = UBS2FunctionLibrary::GetDataSubsystem(this)->GetVehicleDataRow(VehicleStartingData.VehicleID);

	//2. Asynchronously load assets (mesh, anim class)
	TArray<FSoftObjectPath> AssetsToLoad;
	AssetsToLoad.Add(VehicleData->Vehicle_Mesh.ToSoftObjectPath());
	AssetsToLoad.Add(VehicleData->Anim_Class.ToSoftObjectPath());
	
	FStreamableManager* StreamableManager = Cast<UGameInstance_Base>(UGameplayStatics::GetGameInstance(this))->GetStreamableManager();
	StreamableManager->RequestAsyncLoad(AssetsToLoad, FStreamableDelegate::CreateUObject(this, &AVehicle_Base::Init_DetermineVehicleBuildBehavior));
}

void AVehicle_Base::Init_DetermineVehicleBuildBehavior()
{
	if (VehicleStartingData.PreviewVehicle)
	{
		Init_Vehicle_Preview();
	}
	else
	{
		Init_Vehicle();
	}
}

void AVehicle_Base::Init_Vehicle()
{
	Init_VehicleMesh(VehicleData->Vehicle_Mesh.Get());
	Init_EngineAudio();
	Init_Seats();
	VehicleWeaponLogicComponent->Init_VehicleWeaponSystem(VehicleStartingData.StartingVehicleLoadout.SeatLoadout);		//weapons and turrets
	switch (VehicleData->Movement_Type)
	{
		case E_MovementType::GroundVehicle:
			UE_LOG(LogTemp, Log, TEXT("GROUND VEHICLE SETUP STARTED"));
			Init_GroundVehicle();
			Init_Horn(VehicleData->GroundVehicle_Data.HornAudio);
			break;
		case E_MovementType::Helicopter:
			UE_LOG(LogTemp, Log, TEXT("HELICOPTER SETUP STARTED"));
			HandleChaosMovement(false);
			Init_Helicopter();
			break;
		case E_MovementType::Jet:
			UE_LOG(LogTemp, Log, TEXT("JET SETUP STARTED"));
			switch (VehicleData->Jet_Data.FlightModelType)
			{
				case EFlightModelType::Chaos:
					Init_Jet_Chaos();
					break;
				case EFlightModelType::Arcade:
					VehicleMeshComponent->SetSimulatePhysics(false);
					VehicleMeshComponent->SetEnableGravity(false);
					break;
			}
			break;
		case E_MovementType::Boat:
			UE_LOG(LogTemp, Log, TEXT("BOAT SETUP STARTED"));
			break;
	}
	Init_VehicleAnim(VehicleData->Anim_Class.Get());
	ApplyCamoToVehicle(VehicleStartingData.StartingVehicleLoadout.VehicleCamo);
	SpawnComponent->Init_SpawnData(VehicleData->Vehicle_DisplayName, VehicleData->VehicleIcon.Get(), ESpawnType::Vehicle);
	
	Init_Vehicle_Finalize();
}

void AVehicle_Base::Init_Horn(USoundBase* HornAudio)
{
	if (!HornAudio)
	{
		return;
	}

	VehicleCurrentState.GenericVehicleState.HornAudioComponent = NewObject<UAudioComponent>(this, UAudioComponent::StaticClass());
	VehicleCurrentState.GenericVehicleState.HornAudioComponent->RegisterComponent();
	VehicleCurrentState.GenericVehicleState.HornAudioComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	VehicleCurrentState.GenericVehicleState.HornAudioComponent->SetSound(Cast<USoundBase>(HornAudio));
}

void AVehicle_Base::Init_Vehicle_Preview()
{
	Init_VehicleMesh_Preview(VehicleData->Vehicle_Mesh.Get());
	Init_Seats();
	VehicleWeaponLogicComponent->SetComponentTickEnabled(false);
	VehicleWeaponLogicComponent->Init_VehicleWeaponSystem(VehicleStartingData.StartingVehicleLoadout.SeatLoadout);		//weapons and turrets
	ApplyCamoToVehicle(VehicleStartingData.StartingVehicleLoadout.VehicleCamo);
	
	Init_Vehicle_Finalize();
}

void AVehicle_Base::Init_Vehicle_Finalize()
{
	VehicleCurrentState.bIsInitialized = true;
	OnVehicleInitialized.Broadcast();
}

void AVehicle_Base::SetVehicleAndInit(FVehicleStartingData InputVehicleStartingData)
{
	//PROBLEM: with vehicle with things like wheels, this currently isnt clearing that (reason to switch to pooling maybe?)
	UE_LOG(LogTemp, Error, TEXT("VEHICLE REINIT STARTED, SEATS INITIALIZED NOW FALSE"));
	VehicleStartingData = InputVehicleStartingData;
	Init_VehicleData();
}

#pragma endregion

#pragma region Horn

void AVehicle_Base::PlayHorn()
{
	if (!VehicleCurrentState.GenericVehicleState.HornAudioComponent)	{return;}
	VehicleCurrentState.GenericVehicleState.HornAudioComponent->Play(0.0f);
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &AVehicle_Base::StopHorn, 4.0f, false);
}

void AVehicle_Base::StopHorn()
{
	if (!VehicleCurrentState.GenericVehicleState.HornAudioComponent)	{return;}
	VehicleCurrentState.GenericVehicleState.HornAudioComponent->Stop();
}

#pragma endregion

void AVehicle_Base::UpdateSeatList_AllOccupants()
{
	//update seat list/call update seat list function on character side for each character in vehicle
	TArray<AActor*> AttachedActors;
	GetAttachedActors(AttachedActors);
	TArray<ACharacter_Base*> Characters;
	for (AActor* Actor : AttachedActors)
	{
		if (ACharacter_Base* CastedChar = Cast<ACharacter_Base>(Actor))
		{
			Characters.Add(CastedChar);
		}
	}
	for (ACharacter_Base* Character : Characters)
	{
		Character->UpdateSeatList(Characters);
		//call update seat list function on charcter side
		//has ref to vehicle, which means it can pull its state and see if its occupied
		//if need be character side function can even have an input of character array to get characters names, etc
		//this avoids hard coupling characters and vehicles
	}
}

void AVehicle_Base::Interact_Implementation(ACharacter_Base* CharacterInteracting)
{
	if (VehicleData->bCanRemoteControl)		{ return; }		//cant enter an RC Vehicle via in person interaction
	AttemptEnterVehicle(CharacterInteracting);
	InteractionWidgetComponent->SetVisibility(false, false);
}

#pragma region LoadoutUpdates

void AVehicle_Base::ApplyLoadoutToSeat(int32 SeatIndex)		//this functions applies everything in the loadout (weapons, optics, upgrades, camos (if applicable), etc)
{
	//now only called by ApplyLoadoutToVehicle when enter main seat (driver/drivergunner) and is then applied TO EVERY SEAT
	switch (VehicleData->Seats[SeatIndex].SeatRole)
	{
		case E_SeatRole::DriverGunner:
		case E_SeatRole::Gunner:
			VehicleWeaponLogicComponent->ApplySavedWeaponsToSeat(SeatIndex);
			ApplySavedOpticToSeat(SeatIndex);
			break;
	}
}

void AVehicle_Base::ApplySavedOpticToSeat(int32 SeatIndex)
{
	const FSavedSeatLoadout& SeatLoadoutSave = UBS2FunctionLibrary::GetSaveSubsystem(this)->GetSeatLoadout(VehicleData->Vehicle_Type, SeatIndex);

	VehicleCurrentState.SeatStates[SeatIndex].OpticState.CurrentAvailableOptics.Add(SeatLoadoutSave.Optic);
}

void AVehicle_Base::ApplyLoadoutToVehicle()
{
	if (VehicleStartingData.LockLoadout)
	{
		return;
	}

	if (!VehicleCurrentState.GenericVehicleState.LoadoutApplied)
	{
		for (int32 i = 0; i < VehicleData->Seats.Num(); i++)
		{
			ApplyLoadoutToSeat(i);
		}
		ApplyCamoToVehicle(UBS2FunctionLibrary::GetSaveSubsystem(this)->GetVehicleLoadout(VehicleData->Vehicle_Type).VehicleCamo);
		VehicleCurrentState.GenericVehicleState.LoadoutApplied = true;
	}
}

#pragma region ApplyCamo

void AVehicle_Base::ApplyCamoToVehicle(FName CamoID)
{
	for (int32 i = 0; i < VehicleMeshComponent->GetNumMaterials(); i++)
	{
		VehicleMeshComponent->SetMaterial(i, nullptr);
	}

	if (CamoID == NAME_None) return;

	const FVehicleCamoData* CamoDataPtr = VehicleData->AvailableCamos.Find(CamoID);
	if (!CamoDataPtr)
	{
		//here for current crash sake (certain vehicles of type simply dont have a camo that might be saved
		//consider redoing how vehicle loadouts are saved (save by id instead of type)
		return;
	}
	const FVehicleCamoData& CamoData = *CamoDataPtr;
	//apply camo to main body
	for (auto& CamoMeshPair : CamoData.MaterialElementIndexMap)
	{
		int32 MaterialIndex = CamoMeshPair.Key;
		UMaterialInstance* Camo = CamoMeshPair.Value;
		VehicleMeshComponent->CreateDynamicMaterialInstance(MaterialIndex, Camo);
	}
	
	//apply camo to WEAPON attachments (main weapon mesh + decorative attachments)
	for (auto& SeatWeaponSystem : VehicleWeaponLogicComponent->VehicleWeaponSystem)
	{
		FVehicleWeaponSystem_Runtime& WeaponSystem = SeatWeaponSystem.Value;
		//main weapon mesh attachment
		if (SeatWeaponSystem.Value.VehicleWeaponSystemState.WeaponSystemMesh.IsValid())
		{
			ApplyCamoToAttachment(WeaponSystem.VehicleWeaponSystemState.WeaponSystemMesh.Get(), WeaponSystem.VehicleWeaponSystemState.WSAttachmentID, CamoID);
		}

		//weapon decorative attachments
		for (FVehicleWeapon_Runtime& WeaponSlot : WeaponSystem.Weapons)
		{
			for (FDecorative_Runtime& Decorative : WeaponSlot.VehicleWeaponState.VehicleWeaponDecoratives)
			{
				ApplyCamoToAttachment(Decorative.DecorativeMesh.Get(), Decorative.DecorativeAttachmentID, CamoID);
			}
		}
	}
	VehicleCurrentState.GenericVehicleState.CurrentCamo = CamoID;
}

void AVehicle_Base::ApplyCamoToAttachment(TWeakObjectPtr<UMeshComponent> Mesh, FName AttachmentID, FName CamoID)
{
	if (!Mesh.IsValid() || AttachmentID == NAME_None || CamoID == NAME_None) return;

	const FVehicleAttachmentData& AttachmentData = *UBS2FunctionLibrary::GetDataSubsystem(this)->GetVehicleAttachmentDataRow(AttachmentID);
	const FAttachmentCamoData* CamoData = AttachmentData.AvailableCamos.Find(CamoID);
	if (!CamoData)
	{
		// This happens if this specific attachment doesn't have the selected Camo assigned
		return;
	}
	for (const auto& MatPair : CamoData->MaterialElementIndexMap)
	{
		Mesh->CreateDynamicMaterialInstance(MatPair.Key, MatPair.Value);
	}
}

#pragma endregion

#pragma region ClearLoadout

void AVehicle_Base::ClearLoadoutFromSeat(int32 SeatIndex)
{
	VehicleWeaponLogicComponent->ClearWeaponSystemFromSeat(SeatIndex, true);
	//countermeasures
	//optics
	//upgrades
}

void AVehicle_Base::ClearEntireLoadoutFromVehicle()
{
	VehicleWeaponLogicComponent->ClearEntireWeaponSystemFromVehicle();
	//clear camo
	VehicleCurrentState.GenericVehicleState.CurrentCamo = NAME_None;
}

#pragma endregion

#pragma endregion

#pragma region SeatManagement

bool AVehicle_Base::CycleThroughSeats(ACharacter_Base* Character)
{
	int32 TotalSeats = VehicleData->Seats.Num();
	int32& StartIndex = Character->GetCSI();
	int32 Offset = (TotalSeats == 1) ? 0 : 1;

	//try each seat exactly once, skipping the current seat
	for (; Offset < TotalSeats; ++Offset)
	{
		int32 CheckIndex = (StartIndex + Offset) % TotalSeats;		//wraps around the seat list circularly, so if youre at the last seat, it loops back to seat 0.
		if (!VehicleCurrentState.SeatStates[CheckIndex].isOccupied)
		{
			Character->UpdateSeatIndexes(Character->GetCSI(), CheckIndex, CheckIndex);
			return true;
		}
	}
	return false;
}

void AVehicle_Base::HandleViewMethod(ACharacter_Base* Character, const FSeatData& SeatData)
{
	//move to character?
	if (SeatData.SeatRole != E_SeatRole::DriverGunner && SeatData.SeatRole != E_SeatRole::Gunner)
	{
		HandleViewMethod_Default(Character, SeatData);
		return;
	}

	int32 SeatIndex = Character->GetCSI();
	SyncActiveCameraForSeat(SeatIndex);

	UCameraComponent* WeaponCam = GetSeatWeaponCam(SeatIndex);
	if (!WeaponCam)
	{
		HandleViewMethod_Default(Character, SeatData);
		return;
	}
	
	TWeakObjectPtr<AActor> ViewTarget = VehicleWeaponLogicComponent->GetCurrentViewTargetAtSeatIndex(SeatIndex);
	Character->UpdateViewTarget(ViewTarget, WeaponCam);
}

void AVehicle_Base::HandleViewMethod_Default(ACharacter_Base* Character, const FSeatData& SeatData)
{
	//move to character?
	switch (SeatData.ViewMethod)
	{
		case E_ViewMethod::Windowed:
			Character->UpdateViewTarget(Character, Character->FPCamera);
			break;
		case E_ViewMethod::Remote:
		{
			int32 SeatIndex = Character->GetCSI();
			SyncActiveCameraForSeat(SeatIndex);
			Character->UpdateViewTarget(this, VehicleCurrentState.SeatStates[SeatIndex].DefaultCamera);
			break;
		}
	}
}

void AVehicle_Base::HandleSeatOccupationStatus(bool Occupy, int32 SeatIndex)
{
	if (Occupy)
	{
		VehicleCurrentState.SeatStates[SeatIndex].isOccupied = true;
	}
	else
	{
		VehicleCurrentState.SeatStates[SeatIndex].isOccupied = false;
	}
}

void AVehicle_Base::DropSeat(ACharacter_Base* Character, int32& SeatIndex)
{
	//should usually be LSI
	if (SeatIndex == -1)
	{
		SeatIndex = 0;
	}
	const FSeatData& SeatData = VehicleData->Seats[SeatIndex];
	Character->CharacterExitSeat(SeatData.DefaultCharacterContext);
	if (Character->IsLocallyControlled())
	{
		VehicleCurrentState.SeatStates[Character->GetCSI()].UpdateHUD = false;
	}
	HandleSeatOccupationStatus(false, SeatIndex);

	DeactiveSeatCameras(SeatIndex);

	switch (SeatData.SeatRole)
	{
		case E_SeatRole::Driver:
			DropDriver();
			break;
		case E_SeatRole::Gunner:
			DropGunner(Character, SeatIndex);
			break;
		case E_SeatRole::DriverGunner:
			DropDriver();
			DropGunner(Character, SeatIndex);
			break;
	}
}

void AVehicle_Base::SetupNewSeat(ACharacter_Base* Character)
{
	const FSeatData& SeatData = VehicleData->Seats[Character->GetCSI()];
	HandleSeatOccupationStatus(true, Character->GetCSI());

	UpdateSeatList_AllOccupants();

	switch (SeatData.SeatRole)
	{
		case E_SeatRole::Driver:
			ApplyLoadoutToVehicle();
			SetupDriver(Character);
			break;
		case E_SeatRole::Gunner:
			SetupGunner(Character);
			break;
		case E_SeatRole::DriverGunner:
			ApplyLoadoutToVehicle();
			SetupDriver(Character);
			SetupGunner(Character);
			break;
		case E_SeatRole::Passenger:
			break;
	}
	HandleViewMethod(Character, SeatData);

	Character->CharacterEnterSeat(SeatData.DefaultCharacterContext);
	if (VehicleMeshComponent->GetAnimInstance()->GetClass()->ImplementsInterface(UAnims::StaticClass()))
	{
		IAnims::Execute_OnEnterSeat_Vehicle(VehicleMeshComponent->GetAnimInstance(), Character->GetCSI());
	}

	if (Character->IsLocallyControlled())
	{
		VehicleCurrentState.SeatStates[Character->GetCSI()].UpdateHUD = true;
	}
}

#pragma region DriverSeats

void AVehicle_Base::SetupDriver(ACharacter_Base* Character)
{
	//start engine
	UGameplayStatics::PlaySoundAtLocation(this, Cast<USoundBase>(VehicleData->GenericVehicleAudio.EngineStartupAudio), GetActorLocation(), FRotator::ZeroRotator, 1.0f, 1.0f, 0.0f);

	//update engine audio (do sound cue on a timer instead?)
	VehicleCurrentState.GenericVehicleState.EngineAudioComponent->SetSound(Cast<USoundBase>(VehicleData->GenericVehicleAudio.EngineAudio));
	GetWorldTimerManager().SetTimer(TimerHandle_AudioUpdate_Engine, this, &AVehicle_Base::UpdateEngineAudio, 0.05f, true);
	VehicleCurrentState.GenericVehicleState.EngineAudioComponent->Play();
	//engine/whatever else start up audio

	switch (VehicleData->Movement_Type)
	{
		case E_MovementType::GroundVehicle:
			break;
		case E_MovementType::Helicopter:
		{
			GetWorldTimerManager().SetTimer(RotorUpdateTimer, this, &AVehicle_Base::UpdateRotorRPM, 0.016f, true);
			GetWorldTimerManager().SetTimer(VehicleMovementTimer, this, &AVehicle_Base::UpdateMovement_Heli, 0.05f, true);
			break;
		}
		case E_MovementType::Jet:
			break;
	}
}

void AVehicle_Base::DropDriver()
{
	//shutdown engine
	VehicleCurrentState.GenericVehicleState.EngineAudioComponent->Stop();
	GetWorldTimerManager().ClearTimer(TimerHandle_AudioUpdate_Engine);
	UGameplayStatics::PlaySoundAtLocation(this, Cast<USoundBase>(VehicleData->GenericVehicleAudio.EngineShutdownAudio), GetActorLocation(), FRotator::ZeroRotator, 1.0f, 1.0f, 0.0f);

	switch (VehicleData->Movement_Type)
	{
		case E_MovementType::Helicopter:
			GetWorldTimerManager().ClearTimer(VehicleMovementTimer);
			break;
	}
}

#pragma endregion

#pragma region GunnerSeats

void AVehicle_Base::SetupGunner(ACharacter_Base* Character)
{
	//called once on enter of a gunner seat, seat hud should already be on viewport
	const FSeatData& SeatData = VehicleData->Seats[Character->GetCSI()];
	if (SeatData.ViewMethod == E_ViewMethod::Windowed)
	{
		VehicleWeaponLogicComponent->WindowedRangefinder.AddDynamic(Character, &ACharacter_Base::UpdateRangefinder_WindowedVehicle);
	}

	VehicleWeaponLogicComponent->EquipWeapon(Character->GetCSI(), 0);
	VehicleWeaponLogicComponent->GetWAC(Character->GetCSI())->Activate();
}

void AVehicle_Base::DropGunner(TWeakObjectPtr<ACharacter_Base> Character, int32& SeatIndex)
{
	VehicleWeaponLogicComponent->WindowedRangefinder.RemoveDynamic(Character.Get(), &ACharacter_Base::UpdateRangefinder_WindowedVehicle);
	VehicleWeaponLogicComponent->UnequipWeapon(SeatIndex, VehicleWeaponLogicComponent->GetCWIForSeat(SeatIndex), VehicleWeaponLogicComponent->GetEquippedWeaponInSeat(SeatIndex).VehicleWeaponState.BaseWeaponRuntimeData.WeaponState.isFiring);
	VehicleWeaponLogicComponent->GetWAC(SeatIndex)->Deactivate();
}

#pragma endregion

void AVehicle_Base::AttemptEnterVehicle(ACharacter_Base* Character)
{
	bool bFoundSeat = CycleThroughSeats(Character);
	if (bFoundSeat)
	{
		//Enter Vehicle
		Character->ManageinVehicleStatus(this, true);
		SetupNewSeat(Character);
		//do any "enter vehicle" specific stuff (open/close door/ "entering" animation, etc?
	}
}

void AVehicle_Base::ChangeSeat(ACharacter_Base* Character)
{
	bool bFoundSeat = CycleThroughSeats(Character);
	if (bFoundSeat)
	{
		DropSeat(Character, Character->CharacterState.CharacterVehicleState.LSI);
		SetupNewSeat(Character);
	}
}

void AVehicle_Base::SyncActiveCameraForSeat(int32 SeatIndex)
{
	UCameraComponent* WeaponCam = GetSeatWeaponCam(SeatIndex);
	UpdateSeatActiveCamera(SeatIndex, WeaponCam ? WeaponCam : VehicleCurrentState.SeatStates[SeatIndex].DefaultCamera);
}

#pragma 

#pragma region SeatCameraManagement

void AVehicle_Base::UpdateSeatActiveCamera(int32 SeatIndex, UCameraComponent* NewActiveCamera)
{
	FSeatState& SeatState = VehicleCurrentState.SeatStates[SeatIndex];
	if (SeatState.DefaultCamera && SeatState.DefaultCamera != NewActiveCamera)
	{
		SeatState.DefaultCamera->SetActive(false);
	}
	if (UCameraComponent* WeaponCam = GetSeatWeaponCam(SeatIndex); WeaponCam && WeaponCam != NewActiveCamera)
	{
		WeaponCam->SetActive(false);
	}
	NewActiveCamera->SetActive(true);
}

void AVehicle_Base::DeactiveSeatCameras(int32 SeatIndex)
{
	FSeatState& SeatState = VehicleCurrentState.SeatStates[SeatIndex];
	if (SeatState.DefaultCamera)
	{
		SeatState.DefaultCamera->SetActive(false);
	}
	if (UCameraComponent* WeaponCam = GetSeatWeaponCam(SeatIndex))
	{
		WeaponCam->SetActive(false);
	}
}

#pragma endregion

void AVehicle_Base::UpdateEngineAudio()
{
	switch (VehicleData->Movement_Type)
	{
		case E_MovementType::GroundVehicle:
			VehicleCurrentState.GenericVehicleState.EngineAudioComponent->SetFloatParameter(FName("RPM"), FMath::Abs(ChaosVehicleMovement->GetEngineRotationSpeed()));
			VehicleCurrentState.GenericVehicleState.EngineAudioComponent->SetFloatParameter(FName("Speed"), FMath::Abs(ChaosVehicleMovement->GetForwardSpeed() * 0.036f));
			break;
		case E_MovementType::Helicopter:
			//update rotor audio
			VehicleCurrentState.GenericVehicleState.EngineAudioComponent->SetFloatParameter(FName("Volume"), VehicleCurrentState.AircraftState.HelicopterState.RotorRPMs[0]);			//GetVelocity().Size()
			VehicleCurrentState.GenericVehicleState.InteriorAudioComponent->SetFloatParameter(FName("Pitch"), VehicleCurrentState.AircraftState.HelicopterState.RotorRPMs[0]);
			break;
	}
}

void AVehicle_Base::UpdateRotorRPM()
{
	const FHelicopterData& HeliData = VehicleData->Helicopter_Data;

	float& RPM = VehicleCurrentState.AircraftState.HelicopterState.RotorRPMs[0];
	float Alpha = FMath::Clamp(RPM / HeliData.RotorData.TargetRPM, 0.0f, 1.0f);

	float SpeedMult = FMath::InterpEaseIn(
		HeliData.RotorData.MinMaxAccelerationSpeed.GetLowerBoundValue(),
		HeliData.RotorData.MinMaxAccelerationSpeed.GetUpperBoundValue(),
		Alpha,
		HeliData.RotorData.AccelerationCurve
	);

	RPM = FMath::FInterpConstantTo(RPM, HeliData.RotorData.TargetRPM, 0.016f, SpeedMult);

	if (FMath::IsNearlyEqual(RPM, HeliData.RotorData.TargetRPM, 0.1f))
	{
		GetWorld()->GetTimerManager().ClearTimer(RotorUpdateTimer);
	}

	//audio
	//VehicleCurrentState.GenericVehicleState.EngineAudioComponent->SetFloatParameter(FName("EngineRPM"), RPM);
	//v
}

#pragma region MovementInput

void AVehicle_Base::HandleApplyThrottle_GV(float RawInputValue)
{
    const bool bInDeadzone = FMath::IsNearlyZero(RawInputValue, 0.15);		//hardcode for now but should be throttle deadzone
    const bool bGearMidShift = ChaosVehicleMovement->GetTargetGear() != ChaosVehicleMovement->GetCurrentGear();

    if (bInDeadzone || bGearMidShift)
    {
        UpdateThrottle_GV_Stop();
        return; 
    }

    if (ChaosVehicleMovement->GetHandbrakeInput())
    {
        ChaosVehicleMovement->SetHandbrakeInput(false);
    }

    if (RawInputValue > 0.f)
    {
        UpdateThrottle_GV_Forward(RawInputValue);
    }
    else
    {
        UpdateThrottle_GV_Reverse(RawInputValue);
    }
}

void AVehicle_Base::UpdateThrottle_GV_Forward(float InputValue)
{
	switch (ChaosVehicleMovement->GetCurrentGear())
	{
		case -1:
			ChaosVehicleMovement->SetTargetGear(0, true);
			break;
		case 0:
			ChaosVehicleMovement->SetTargetGear(1, true);
			break;
	}

	ChaosVehicleMovement->SetThrottleInput(InputValue);
}

void AVehicle_Base::UpdateThrottle_GV_Reverse(float RawInputValue)
{
	float ThrottleValue = FMath::Abs(RawInputValue);
	switch (ChaosVehicleMovement->GetCurrentGear())
	{
		case 0:
			ChaosVehicleMovement->SetTargetGear(-1, true);
			break;
		case 1:
			ChaosVehicleMovement->SetTargetGear(0, true);
			break;
	}
	ChaosVehicleMovement->SetBrakeInput(ThrottleValue);
	ChaosVehicleMovement->SetThrottleInput(0.0f);
}

void AVehicle_Base::UpdateThrottle_GV_Stop()
{
	ChaosVehicleMovement->SetTargetGear(0, true);
	ChaosVehicleMovement->SetHandbrakeInput(true);
	ChaosVehicleMovement->SetThrottleInput(0.0f);
	ChaosVehicleMovement->SetBrakeInput(0.0f);

	GetWorld()->GetTimerManager().SetTimer(SpeedTimer, [this]()
	{
		UBS2FunctionLibrary::GetHUDSubsystem(this)->UpdateSpeedHUD_Vehicle(GetVelocity().Size());		//block behind updatehud check?
	}, 0.05f, true);
	if (GetVelocity().Size() <= 0.0f)
	{
		GetWorld()->GetTimerManager().ClearTimer(SpeedTimer);
	}
}

void AVehicle_Base::Input_UpdateSteering_GV(float SteeringValue, int32 SeatIndex)
{
	if (VehicleData->GroundVehicle_Data.IdleTurnData.canIdleTurn)
	{
		// 1. Get the current turning speed of the vehicle mesh (radians per second)
		FVector CurrentAngularVelocity = VehicleMeshComponent->GetPhysicsAngularVelocityInRadians();
		float CurrentYawSpeed = CurrentAngularVelocity.Z;

		// 2. Tune these two parameters (ideally expose these to your VehicleData asset)
		const float& TargetTorqueScale = VehicleData->GroundVehicle_Data.IdleTurnData.TargetTorqueScale;  // Power of the turn (increased since drag will fight it)
		const float& AngularDragScale = VehicleData->GroundVehicle_Data.IdleTurnData.AngularDragScale;   // How hard the tank fights its own rotation speed

		// 3. Calculate the forces
		// The driver wants to accelerate rotation:
		float DrivingTorque = SteeringValue * TargetTorqueScale;

		// The counter-force that grows stronger the faster the tank is already spinning:
		float DampingTorque = -CurrentYawSpeed * AngularDragScale;

		// Combine them so they fight each other
		float FinalYawTorque = DrivingTorque + DampingTorque;

		FVector TorqueVector = FVector(0.0f, 0.0f, FinalYawTorque);

		// Inject the balanced torque onto the mesh
		VehicleMeshComponent->AddTorqueInRadians(TorqueVector, NAME_None, true);
	}
	else
	{
		ChaosVehicleMovement->SetSteeringInput(SteeringValue);
	}
	if (UCameraComponent* ActiveCam = GetRemoteActiveCam(SeatIndex))
	{
		UBS2FunctionLibrary::GetHUDSubsystem(this)->UpdateCompassHUD_Vehicle(ActiveCam->GetComponentRotation().Yaw);
	}
}

#pragma region HeliInput

void AVehicle_Base::UpdateThrottle_Heli(float InputValue)
{
	float& CurrentHoverVelocity = VehicleCurrentState.AircraftState.HelicopterState.CurrentHoverVelocity;
	const FHelicopterData& HeliData = VehicleData->Helicopter_Data;
	if (InputValue != 0.0f)
	{
		const float Target = InputValue * HeliData.MaxThrust;
		CurrentHoverVelocity = FMath::FInterpTo(CurrentHoverVelocity, Target, GetWorld()->GetDeltaSeconds(), 2.0f);
	}
	else
	{
		CurrentHoverVelocity = FMath::FInterpTo(CurrentHoverVelocity, HeliData.HoverPower, GetWorld()->GetDeltaSeconds(), 4.0f);
	}
	const FVector UpVel = GetActorUpVector() * CurrentHoverVelocity;
	VehicleMeshComponent->SetAllPhysicsLinearVelocity(UpVel, true);
}

void AVehicle_Base::UpdatePitch_Heli(float InputValue)
{
	float& CurrentPitchSpeed = VehicleCurrentState.AircraftState.HelicopterState.CurrentPitchSpeed;
	const FHelicopterData& HeliData = VehicleData->Helicopter_Data;

	const float Target = InputValue * HeliData.MaxPitchSpeed;
	CurrentPitchSpeed = FMath::FInterpTo(CurrentPitchSpeed, Target, GetWorld()->GetDeltaSeconds(), 2.0f);

	AddActorLocalRotation(FRotator(CurrentPitchSpeed, 0.0f, 0.0f), false, nullptr, ETeleportType::TeleportPhysics);

	UBS2FunctionLibrary::GetHUDSubsystem(this)->UpdateAltitudeHUD_Vehicle(GetActorRotation().Pitch);
}

void AVehicle_Base::UpdateYaw_Heli(float InputValue)
{
	FHelicopterState& HeliState = VehicleCurrentState.AircraftState.HelicopterState;
	float& CurrentYawSpeed = HeliState.CurrentYawSpeed;
	const FHelicopterData& HeliData = VehicleData->Helicopter_Data;

	CurrentYawSpeed = FMath::FInterpTo(CurrentYawSpeed, InputValue * HeliData.MaxYawSpeed, GetWorld()->GetDeltaSeconds(), 2.0f);
	// Yaw is world rotation, scaled by DeltaSeconds * 100 (matching BP)
	const float YawThisFrame = HeliState.CurrentYawSpeed * GetWorld()->GetDeltaSeconds() * 100.0f;
	AddActorWorldRotation(FRotator(0.0f, YawThisFrame, 0.0f), false, nullptr, ETeleportType::TeleportPhysics);
}

void AVehicle_Base::UpdateRoll_Heli(float InputValue)
{
	FHelicopterState& HeliState = VehicleCurrentState.AircraftState.HelicopterState;
	float& CurrentRollSpeed = HeliState.CurrentRollSpeed;
	const FHelicopterData& HeliData = VehicleData->Helicopter_Data;

	CurrentRollSpeed = FMath::FInterpTo(CurrentRollSpeed, InputValue * HeliData.MaxRollSpeed, GetWorld()->GetDeltaSeconds(), 2.0f);
	AddActorLocalRotation(FRotator(0.0f, 0.0f, CurrentRollSpeed), false, nullptr, ETeleportType::TeleportPhysics);

	//turning bleeds/adds momentum (drift mechanic from EA3D)
	const float MomentumAdjust = (HeliState.CurrentForwardMomentum * 2.0f) + InputValue;
	const float Target = FMath::Clamp(MomentumAdjust, -50.0f, 50.0f);
	HeliState.CurrentForwardMomentum = FMath::FInterpTo(HeliState.CurrentForwardMomentum, Target, GetWorld()->GetDeltaSeconds(), 2.0f);
}

void AVehicle_Base::UpdateMovement_Heli()
{
	auto& State = VehicleCurrentState.AircraftState.HelicopterState;
	const auto& Data = VehicleData->Helicopter_Data;
	const float DeltaTime = GetWorld()->GetDeltaSeconds();

	//update momentum heli
	const float PitchDeg = FMath::Clamp(GetActorRotation().Pitch, -50.0f, 50.0f);
	const float NormalizedPitch = (PitchDeg + 50.0f) / 100.0f; // NormalizeToRange(-50, 50, 0, 1)

	const float CurveValue = Data.PitchCurve ? Data.PitchCurve->GetFloatValue(NormalizedPitch) : NormalizedPitch;

	State.CurrentForwardMomentum += CurveValue * Data.Acceleration * DeltaTime;
	State.CurrentForwardMomentum = FMath::Clamp(State.CurrentForwardMomentum, Data.MinMomentum, Data.MaxMomentum);

	//calculate target velocity from momentum
	// Forward component: actor yaw only (strip pitch/roll from direction)
	const FRotator YawOnly(0.0f, GetActorRotation().Yaw, 0.0f);
	const FVector ForwardDir = YawOnly.Vector();
	const FVector RightDir = FRotationMatrix(YawOnly).GetScaledAxis(EAxis::Y);
	// Forward: pitch * -1 (nose-down = positive thrust along yaw direction)
	const FVector ForwardComp = ForwardDir * State.CurrentForwardMomentum * (PitchDeg * -1.0f);
	// Right: momentum scaled by roll angle (bank = lateral drift)
	const FVector RightComp = RightDir * State.CurrentForwardMomentum * GetActorRotation().Roll;
	const FVector UpComp = GetActorUpVector() * State.CurrentHoverVelocity;
	const FVector TargetVel = ForwardComp + RightComp + UpComp;
	// Smooth velocity interpolation
	const FVector CurrentVel = VehicleMeshComponent->GetPhysicsLinearVelocity();
	const FVector NewVel = FMath::VInterpTo(CurrentVel, TargetVel, DeltaTime, 3.0f);

	VehicleMeshComponent->SetPhysicsLinearVelocity(NewVel, false, NAME_None);

	//gravity when upside down
	const FVector Up = VehicleMeshComponent->GetUpVector();
	if (Up.Z < -0.75f)
	{
		const float Alpha = FMath::GetMappedRangeValueClamped(FVector2D(-1.0f, 0.0f), FVector2D(-1.0f, 0.0f), Up.Z);
		const float GravityImpulse = Alpha * Data.Gravity;

		VehicleMeshComponent->AddImpulse(FVector(0.0f, 0.0f, GravityImpulse), NAME_None, true);
	}

	LimitHeliSpeed();
}

void AVehicle_Base::LimitHeliSpeed()
{
	const auto& Data = VehicleData->Helicopter_Data;

	// Speed cap
	FVector Vel = GetVelocity();
	if (Vel.Size() > Data.MaxVelocity)
	{
		Vel = Vel.GetClampedToMaxSize(Data.MaxVelocity);
		VehicleMeshComponent->SetAllPhysicsLinearVelocity(Vel, false);
	}
}

#pragma endregion

#pragma region JetInput

void AVehicle_Base::HandleThrottleInput_Jet(float InputValue)
{
	auto& AircraftState = VehicleCurrentState.AircraftState;
	auto& JetState = AircraftState.JetState;
	auto& JetData = VehicleData->Jet_Data;
	JetState.CurrentThrottle = InputValue;
	float UnclampedThrust;

	switch (JetData.FlightModelType)
	{
		case EFlightModelType::Arcade:
			UnclampedThrust = (InputValue * GetWorld()->GetDeltaSeconds() * JetData.JetFlightData_Arcade.ThrustMultiplier) + JetState.CurrentThrust;
			JetState.CurrentThrust = FMath::Clamp(UnclampedThrust, 0, JetData.JetFlightData_Arcade.MaxThrustSpeed);
			if (!GetWorldTimerManager().IsTimerActive(ThrottleTimer))
			{
				GetWorldTimerManager().SetTimer(ThrottleTimer, this, &AVehicle_Base::UpdateThrottle_Jet_Arcade, GetWorld()->GetDeltaSeconds(), true);
			}
			break;
		case EFlightModelType::Chaos:
			if (!GetWorldTimerManager().IsTimerActive(ThrottleTimer))
			{
				GetWorldTimerManager().SetTimer(ThrottleTimer, this, &AVehicle_Base::UpdateThrottle_Jet_Chaos, GetWorld()->GetDeltaSeconds(), true);
			}
			break;
	}
}

void AVehicle_Base::HandlePitchInput_Jet(float InputValue)
{
	auto& AircraftState = VehicleCurrentState.AircraftState;
	auto& JetState = AircraftState.JetState;
	auto& JetData = VehicleData->Jet_Data;
	switch (JetData.FlightModelType)
	{
		case EFlightModelType::Arcade:
			UpdatePitch_Jet_Arcade(InputValue);
			break;
		case EFlightModelType::Chaos:
			UpdatePitch_Jet_Chaos(InputValue);
			break;
	}
}

void AVehicle_Base::HandleRollInput_Jet(float InputValue)
{
	auto& AircraftState = VehicleCurrentState.AircraftState;
	auto& JetState = AircraftState.JetState;
	auto& JetData = VehicleData->Jet_Data;
	switch (JetData.FlightModelType)
	{
		case EFlightModelType::Arcade:
			UpdateRoll_Jet_Arcade(InputValue);
			break;
		case EFlightModelType::Chaos:
			UpdateRoll_Jet_Chaos(InputValue);
			break;
	}
}

void AVehicle_Base::UpdateThrottle_Jet_Chaos()
{
	auto& AircraftState = VehicleCurrentState.AircraftState;
	auto& JetState = AircraftState.JetState;
	JetState.CurrentThrust = FMath::FInterpTo(JetState.CurrentThrust, JetState.CurrentThrottle, GetWorld()->GetDeltaSeconds(), VehicleData->Jet_Data.JetFlightData_Chaos.ThrottleInterpSpeed);

	//do landing gear bool to select divide thruster by value here

	UpdateThruster_Chaos(JetState.CurrentThrust);
	if (JetState.CurrentThrottle == JetState.CurrentThrust)
	{
		GetWorldTimerManager().ClearTimer(ThrottleTimer);
	}
}

void AVehicle_Base::UpdateThruster_Chaos(float Throttle)
{
	auto& AircraftState = VehicleCurrentState.AircraftState;
	//ChaosVehicleMovement->SetThrottleInput(JetState.CurrentThrottle / GearDivider);
	ChaosVehicleMovement->SetThrottleInput(Throttle);

	if (Throttle != -1.0f && !AircraftState.bLandingGearIsDown)
	{
		ChaosVehicleMovement->SetHandbrakeInput(false);
	}
	else
	{
		ChaosVehicleMovement->SetHandbrakeInput(true);
	}
}

void AVehicle_Base::UpdatePitch_Jet_Chaos(float InputValue)
{
	auto& AircraftState = VehicleCurrentState.AircraftState;
	auto& JetState = AircraftState.JetState;
	const auto& JetData = VehicleData->Jet_Data;
	const float DeltaTime = GetWorld()->GetDeltaSeconds();
	const float& CurrentSpeed = GetCurrentSpeed_Chaos();

	//const bool bSlow = (CurrentSpeed <= JetData.JetFlightData_Arcade.ControlAuthoritySpeedThreshold);

	const float ClampValue = JetData.JetFlightData_Chaos.PitchClampCurve->GetFloatValue(CurrentSpeed);

	JetState.CurrentPitch = FMath::Clamp(InputValue, -0.6, 0.6);

	JetState.CurrentElevator = FMath::FInterpTo(JetState.CurrentElevator, JetState.CurrentPitch, DeltaTime, JetData.JetFlightData_Chaos.ControlInterpSpeed);

	if (CurrentSpeed > JetData.JetFlightData_Chaos.TakeoffVelocity)
	{
		ChaosVehicleMovement->SetPitchInput(JetState.CurrentElevator);
	}
}

void AVehicle_Base::UpdateRoll_Jet_Chaos(float InputValue)
{
	auto& AircraftState = VehicleCurrentState.AircraftState;
	auto& JetState = AircraftState.JetState;
	const auto& JetData = VehicleData->Jet_Data;
	const float DeltaTime = GetWorld()->GetDeltaSeconds();

	JetState.CurrentRoll = InputValue;

	JetState.CurrentFlaperon = FMath::FInterpTo(JetState.CurrentFlaperon, InputValue, DeltaTime, JetData.JetFlightData_Chaos.ControlInterpSpeed);

	const float Effectiveness = FMath::GetMappedRangeValueClamped(FVector2D(0.0f, JetData.JetFlightData_Chaos.FlaperonFullEffectSpeed), FVector2D(0.0f, 1.0f), GetCurrentSpeed_Chaos());

	const float RollInput = FMath::Clamp(JetState.CurrentFlaperon * Effectiveness, -1.0f, 1.0f);

	ChaosVehicleMovement->SetRollInput(RollInput);
}

void AVehicle_Base::UpdateYaw_Jet_Chaos(float InputValue)
{
	auto& AircraftState = VehicleCurrentState.AircraftState;
	auto& JetState = AircraftState.JetState;
	const auto& JetData = VehicleData->Jet_Data;
	const float DeltaTime = GetWorld()->GetDeltaSeconds();
	const float CurrentSpeed = GetVelocity().Size();

	JetState.CurrentYaw = InputValue;

	JetState.CurrentRudder = FMath::FInterpTo(JetState.CurrentRudder, InputValue, DeltaTime, JetData.JetFlightData_Chaos.ControlInterpSpeed);

	const bool bAirborne = CurrentSpeed > JetData.JetFlightData_Chaos.TakeoffVelocity;

	if (bAirborne)
	{
		const bool bSlow = (CurrentSpeed <= JetData.JetFlightData_Chaos.ControlAuthoritySpeedThreshold);
		const float YawClamp = bSlow ? JetData.JetFlightData_Chaos.YawMax_Slow : 1.0f;

		const float YawInput = FMath::Clamp(InputValue, -YawClamp, YawClamp);
		ChaosVehicleMovement->SetYawInput(YawInput);
	}
	else
	{
		const float RudderInput = FMath::Clamp(JetState.CurrentRudder, -JetData.JetFlightData_Chaos.RudderGroundClamp, JetData.JetFlightData_Chaos.RudderGroundClamp);
		ChaosVehicleMovement->SetYawInput(RudderInput);
	}
}

void AVehicle_Base::AutoLevel_Jet()
{
}

void AVehicle_Base::UpdateThrottle_Jet_Arcade()
{
	const FJetFlightModel_Arcade& FlightModelData = VehicleData->Jet_Data.JetFlightData_Arcade;
	auto& AircraftState = VehicleCurrentState.AircraftState;
	auto& JetState = AircraftState.JetState;

	float NewCurrentSpeed = FMath::FInterpTo(JetState.CurrentSpeed, JetState.CurrentThrust, GetWorld()->GetDeltaSeconds(), FlightModelData.ThrustDrag);
	if (JetState.CurrentThrust < JetState.CurrentSpeed)
	{
		JetState.CurrentSpeed = NewCurrentSpeed;
	}
	else
	{
		JetState.CurrentSpeed = JetState.CurrentThrust;
	}

	FVector NewPosition = ((JetState.CurrentSpeed * GetWorld()->GetDeltaSeconds()) * GetActorForwardVector());
	float Gravity = 981.0f;
	float AppliedGravity = FMath::GetMappedRangeValueClamped(FVector2D(0.0f, FlightModelData.TakeOffSpeedReq), FVector2D(Gravity, 0.0f), JetState.CurrentSpeed);
	float NewZ = NewPosition.Z - (AppliedGravity * GetWorld()->GetDeltaSeconds());
	FVector DeltaLocation = FVector(NewPosition.X, NewPosition.Y, NewZ);
	bool bSweep = true;
	FHitResult* HitResult = nullptr;
	AddActorWorldOffset(DeltaLocation, bSweep, HitResult, ETeleportType::None);
}

void AVehicle_Base::UpdatePitch_Jet_Arcade(float InputValue)
{
	const auto& JetData = VehicleData->Jet_Data;
	auto& AircraftState = VehicleCurrentState.AircraftState;
	auto& JetState = AircraftState.JetState;
	if (JetState.CurrentThrust < JetData.JetFlightData_Arcade.TakeOffSpeedReq) { return; }

	float TargetPitch = InputValue;

	JetState.CurrentPitch = FMath::FInterpTo(JetState.CurrentPitch, TargetPitch, GetWorld()->GetDeltaSeconds(), JetData.JetFlightData_Arcade.PitchSpeed);
	float NewDeltaPitch = JetState.CurrentPitch * GetWorld()->GetDeltaSeconds() * JetData.JetFlightData_Arcade.PitchMultiplier;

	AddActorLocalRotation(FRotator(NewDeltaPitch, 0.0f, 0.0f));
}

void AVehicle_Base::UpdateRoll_Jet_Arcade(float InputValue)
{
	const auto& JetData = VehicleData->Jet_Data;
	auto& AircraftState = VehicleCurrentState.AircraftState;
	auto& JetState = AircraftState.JetState;

	float TargetRoll = InputValue;

	JetState.CurrentRoll = FMath::FInterpTo(JetState.CurrentRoll, TargetRoll, GetWorld()->GetDeltaSeconds(), JetData.JetFlightData_Arcade.RollSpeed);
	float NewDeltaRoll = JetState.CurrentRoll * GetWorld()->GetDeltaSeconds() * JetData.JetFlightData_Arcade.RollMultipler;

	AddActorLocalRotation(FRotator(0.0f, 0.0f, NewDeltaRoll));
}

#pragma endregion

void AVehicle_Base::Input_HandleApplyThrottle(float ThrottleValue)
{
	const E_MovementType& MovementType = VehicleData->Movement_Type;
	switch (MovementType)
	{
		case E_MovementType::GroundVehicle:
			HandleApplyThrottle_GV(ThrottleValue);
			break;
		case E_MovementType::Helicopter:
			UpdateThrottle_Heli(ThrottleValue);
			break;
		case E_MovementType::Jet:
			HandleThrottleInput_Jet(ThrottleValue);
			break;
	}
}

void AVehicle_Base::Input_HandleReleaseThrottle()
{
	const E_MovementType& MovementType = VehicleData->Movement_Type;
	switch (MovementType)
	{
		case E_MovementType::GroundVehicle:
			UpdateThrottle_GV_Stop();
			break;
	}
}

#pragma endregion

#pragma region Optics

void AVehicle_Base::ToggleOptic(int32 SeatIndex)
{
	FOpticState& OpticState = VehicleCurrentState.SeatStates[SeatIndex].OpticState;
	int32 PreviousOpticIndex = OpticState.CurrentOpticIndex;
	UBS2FunctionLibrary::UpdateOpticIndex(OpticState.CurrentAvailableOptics.Num(), OpticState.CurrentOpticIndex);
	if (PreviousOpticIndex == OpticState.CurrentOpticIndex) { return;  }

	const FOpticData* CurrentOpticData = UBS2FunctionLibrary::GetDataSubsystem(this)->GetOpticDataRow(OpticState.CurrentAvailableOptics[OpticState.CurrentOpticIndex]);
	const FOpticData& PreviousOpticData = *UBS2FunctionLibrary::GetDataSubsystem(this)->GetOpticDataRow(OpticState.CurrentAvailableOptics[PreviousOpticIndex]);
	if (!CurrentOpticData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Vehicle_Base::ToggleOptic] - Invalid Optic Data for Optic ID: %s"), *OpticState.CurrentAvailableOptics[OpticState.CurrentOpticIndex].ToString());
		return;
	}
	//HandleTogglePPOptic
	if (CurrentOpticData->OpticPPSettings.WeightedBlendables.Array.Num() > 0)
	{
		TurnOnPPOptic(SeatIndex);
	}
	else if (OpticState.isOn)
	{
		TurnOffPPOptic(SeatIndex, PreviousOpticIndex);
	}

	ToggleMagnificationOptic(SeatIndex, CurrentOpticData->ZoomMagnification);
}

void AVehicle_Base::TurnOnPPOptic(int32 SeatIndex)
{
	FOpticState& OpticState = VehicleCurrentState.SeatStates[SeatIndex].OpticState;
	const FOpticData& CurrentOpticData = *UBS2FunctionLibrary::GetDataSubsystem(this)->GetOpticDataRow(OpticState.CurrentAvailableOptics[OpticState.CurrentOpticIndex]);
	UpdateRemoteActiveCamPP(SeatIndex, UBS2FunctionLibrary::GetDataSubsystem(this)->GetOpticDataRow(OpticState.CurrentAvailableOptics[OpticState.CurrentOpticIndex])->OpticPPSettings, 1.0f, GetRemoteActiveCam(SeatIndex));
	OpticState.isOn = true;
	UGameplayStatics::PlaySound2D(GetWorld(), CurrentOpticData.PowerOnSound);
	if (VehicleCurrentState.SeatStates[SeatIndex].UpdateHUD)
	{
		UBS2FunctionLibrary::GetHUDSubsystem(this)->UpdaticOpticNameHUD_Vehicle(CurrentOpticData.OpticDisplayNameAbrev);
		if (CurrentOpticData.InverseUIColor.A > 0)
		{
			UBS2FunctionLibrary::GetHUDSubsystem(this)->UpdateVehicleHUD_Color(CurrentOpticData.InverseUIColor);
		}
	}
}

void AVehicle_Base::TurnOffPPOptic(int32 SeatIndex, int32 PreviousOpticIndex)
{
	FOpticState& OpticState = VehicleCurrentState.SeatStates[SeatIndex].OpticState;
	const FOpticData& CurrentOpticData = *UBS2FunctionLibrary::GetDataSubsystem(this)->GetOpticDataRow(OpticState.CurrentAvailableOptics[OpticState.CurrentOpticIndex]);
	const FOpticData& PreviousOpticData = *UBS2FunctionLibrary::GetDataSubsystem(this)->GetOpticDataRow(OpticState.CurrentAvailableOptics[PreviousOpticIndex]);
	UpdateRemoteActiveCamPP(SeatIndex, FPostProcessSettings(), 0.0f, GetRemoteActiveCam(SeatIndex));
	OpticState.isOn = false;
	UGameplayStatics::PlaySound2D(GetWorld(), PreviousOpticData.PowerOffSound);
	if (VehicleCurrentState.SeatStates[SeatIndex].UpdateHUD)
	{
		UBS2FunctionLibrary::GetHUDSubsystem(this)->UpdaticOpticNameHUD_Vehicle(CurrentOpticData.OpticDisplayNameAbrev);
		if (PreviousOpticData.InverseUIColor.A > 0)
		{

		}
		if (CurrentOpticData.InverseUIColor.A == 0)
		{
			UBS2FunctionLibrary::GetHUDSubsystem(this)->UpdateVehicleHUD_Color(FLinearColor(0.0f, 1.0f, 0.036889f, 1.0f));		//HARD CODED FOR NOW, CHANGE TO BE TAKEN FROM HUD/UI SETTINGS
		}
	}
}

void AVehicle_Base::ToggleMagnificationOptic(int32 SeatIndex, float ZoomMagnification)
{
	//when change 90 to default data, put it behind an if to see if cam's current fov = default fov
	FOpticState& OpticState = VehicleCurrentState.SeatStates[SeatIndex].OpticState;
	float& CurrentFOV = GetRemoteActiveCam(SeatIndex)->FieldOfView;
	float ReticleScale = VehicleWeaponLogicComponent->GetVWID(SeatIndex, VehicleWeaponLogicComponent->GetCWIForSeat(SeatIndex), VehicleWeaponLogicComponent->GetEquippedWeaponInSeat(SeatIndex).VehicleWeaponState.BaseWeaponRuntimeData.WeaponID).WeaponUIInstanceData.ReticleScale;

	if (ZoomMagnification != 1.0f)
	{
		//zoom optic
		float NewFOV = CurrentFOV / ZoomMagnification;			//CHANGE TO DEFAULT DATA 	
		if (!OpticState.isMagnified)
		{
			const FOpticData& CurrentOpticData = *UBS2FunctionLibrary::GetDataSubsystem(this)->GetOpticDataRow(OpticState.CurrentAvailableOptics[OpticState.CurrentOpticIndex]);
			CurrentFOV = NewFOV;
			OpticState.isMagnified = true;
			UGameplayStatics::PlaySound2D(GetWorld(), CurrentOpticData.PowerOnSound);
			if (VehicleCurrentState.SeatStates[SeatIndex].UpdateHUD)
			{
				
				UBS2FunctionLibrary::GetHUDSubsystem(this)->UpdateOpticMagnificationHUD_Vehicle(ZoomMagnification);
				UBS2FunctionLibrary::GetHUDSubsystem(this)->UpdateWeaponReticleSize_Vehicle(ZoomMagnification * ReticleScale * 0.75f);
			}
		}
	}
	//unzoom optic
	else if (OpticState.isMagnified)
	{
		//WORK ON THIS
		if (!OpticState.isOn)
		{
			const FOpticData& CurrentOpticData = *UBS2FunctionLibrary::GetDataSubsystem(this)->GetOpticDataRow(OpticState.CurrentAvailableOptics[OpticState.CurrentOpticIndex]);
			CurrentFOV = 90.0f;
			OpticState.isMagnified = false;
			UGameplayStatics::PlaySound2D(GetWorld(), CurrentOpticData.PowerOnSound);
			if (VehicleCurrentState.SeatStates[SeatIndex].UpdateHUD)
			{
				UBS2FunctionLibrary::GetHUDSubsystem(this)->UpdateOpticMagnificationHUD_Vehicle(ZoomMagnification);
				UBS2FunctionLibrary::GetHUDSubsystem(this)->UpdateWeaponReticleSize_Vehicle(ReticleScale);
			}
		}
	}
}

void AVehicle_Base::UpdateRemoteActiveCamPP(int32 SeatIndex, FPostProcessSettings PPSettings, float BlendWeight, UCameraComponent* Cam)
{
	Cam->PostProcessSettings = PPSettings;
	Cam->PostProcessBlendWeight = BlendWeight;
}

#pragma endregion

int32 AVehicle_Base::GetControlledTurret(int32 SeatIndex)
{
	return VehicleData->Seats[SeatIndex].AvailableItems.ControlledTurretIndexes[0];
}

UCameraComponent* AVehicle_Base::GetSeatWeaponCam(int32 SeatIndex)
{
	const FSeatData& SeatData = VehicleData->Seats[SeatIndex];
	if (SeatData.SeatRole != E_SeatRole::Gunner && SeatData.SeatRole != E_SeatRole::DriverGunner)
	{
		return nullptr;
	}
	FVehicleWeaponSystem_Runtime* WeaponSystem = VehicleWeaponLogicComponent->VehicleWeaponSystem.Find(SeatIndex);
	if (!WeaponSystem)
	{
		return nullptr;   // weapon system for this seat hasn't been set up yet
	}

	int32 CWI = VehicleWeaponLogicComponent->GetCWIForSeat(SeatIndex);
	if (!WeaponSystem->Weapons.IsValidIndex(CWI))
	{
		return nullptr;
	}

	FVehicleWeapon_Runtime& CurrentWeapon = VehicleWeaponLogicComponent->VehicleWeaponSystem.Find(SeatIndex)->Weapons[CWI];
	const FVehicleWeaponInstanceData& VWID = VehicleWeaponLogicComponent->GetVWID(SeatIndex, CWI, CurrentWeapon.VehicleWeaponState.BaseWeaponRuntimeData.WeaponID);
	return VWID.bHasSpecialCam ? CurrentWeapon.VehicleWeaponState.WeaponTurretCamera : nullptr;
}

UCameraComponent* AVehicle_Base::GetRemoteActiveCam(int32 SeatIndex)
{
	UCameraComponent* WeaponCam = GetSeatWeaponCam(SeatIndex);
	return WeaponCam ? WeaponCam : VehicleCurrentState.SeatStates[SeatIndex].DefaultCamera;
}

float AVehicle_Base::GetCurrentSpeed_Chaos()
{
	//returns KM/H
	return FMath::Floor(FMath::Abs(ChaosVehicleMovement->GetForwardSpeed() * 0.036f));
}

USkeletalMeshComponent* AVehicle_Base::GetMesh() const
{
	return VehicleMeshComponent;
}

FName AVehicle_Base::GetVehicleID() const
{
	return VehicleStartingData.VehicleID;
}

const FVehicleData& AVehicle_Base::GetVehicleData() const
{
	return *VehicleData;
}

const FVehicleCurrentState& AVehicle_Base::GetVehicleState() const
{
	return VehicleCurrentState;
}

AVehicle_Base& AVehicle_Base::GetVehicle() 
{
	return *this;
}

void AVehicle_Base::GetInteractObjectInfo_Implementation(FText& ObjectName, TSoftObjectPtr<UTexture2D>& ObjectIcon)
{
	ObjectName = VehicleData->Vehicle_DisplayName;
	ObjectIcon = VehicleData->VehicleIcon;
}

bool AVehicle_Base::GetIfCanLockOn_Implementation(const TArray<ETargetingCategory>& TargetingCategories, EHomingCapability HomingCapability)
{
	switch (HomingCapability)
	{
		case EHomingCapability::NoHoming:
			return false;
		case EHomingCapability::WireGuided1:
			break;
		case EHomingCapability::WireGuided2:	//(the 1 that allows for some form of homing)
		case EHomingCapability::CanLockOn:
		case EHomingCapability::RequireLockOn:
			for (ETargetingCategory TargetingCategory : TargetingCategories)
			{
				switch (TargetingCategory)
				{
					case ETargetingCategory::GroundVehicle:
						if (VehicleData->Movement_Type == E_MovementType::GroundVehicle || VehicleData->Movement_Type == E_MovementType::Boat)
						{
							return true;
						}
						break;
					case ETargetingCategory::Aircraft:
						if (VehicleData->Movement_Type == E_MovementType::Helicopter || VehicleData->Movement_Type == E_MovementType::Jet)
						{
							return true;
						}
						break;
					case ETargetingCategory::LazedTarget:
						//if this vehicle is currently lazed
						break;
					}
			}
			return false;
	}
	return false;
}





