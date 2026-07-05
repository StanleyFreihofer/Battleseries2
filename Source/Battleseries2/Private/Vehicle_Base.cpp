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

void AVehicle_Base::Init_Wheels(const TArray<FChaosWheelSetup>& WheelData)
{
	ChaosVehicleMovement->WheelSetups.Empty();
	ChaosVehicleMovement->Wheels.Empty();

	ChaosVehicleMovement->WheelSetups.SetNum(WheelData.Num());
	for (int32 i = 0; i < WheelData.Num(); i++)
	{
		const FChaosWheelSetup& SourceData = WheelData[i];				//the data we are pulling from
		FChaosWheelSetup& Setup = ChaosVehicleMovement->WheelSetups[i];	//the properties we are applying to
		Setup.WheelClass = SourceData.WheelClass;
		Setup.BoneName = SourceData.BoneName;							//set bone name
		Setup.AdditionalOffset = SourceData.AdditionalOffset;			//set additional offset
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
	Init_Wheels(VehicleData->GroundVehicle_Data.WheelData);

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
			NewCamera = SpawnAndAttachCamera(CameraSocketName, VehicleMeshComponent);
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
			Init_Wheels(VehicleData->Aircraft_Data.LandingGear);
			HandleChaosMovement(true);
			ChaosVehicleMovement->SetUpdatedComponent(VehicleMeshComponent);
			ChaosVehicleMovement->Aerofoils = VehicleData->Jet_Data.ChaosFlightModel.Aerofoils;
			ChaosVehicleMovement->Thrusters = VehicleData->Jet_Data.ChaosFlightModel.Thrusters;
			ChaosVehicleMovement->TorqueControl = VehicleData->Jet_Data.ChaosFlightModel.TorqueControl;
			ChaosVehicleMovement->TargetRotationControl = VehicleData->Jet_Data.ChaosFlightModel.TargetRotationControl;
			ChaosVehicleMovement->StabilizeControl = VehicleData->Jet_Data.ChaosFlightModel.StabilizeControl;
			Init_Chaos_VehicleSetup(VehicleData->Jet_Data.VehicleSetup);
			ChaosVehicleMovement->RegisterComponent();
			ChaosVehicleMovement->RecreatePhysicsState();;
			VehicleMeshComponent->InitAnim(true);
			break;
		case E_MovementType::Boat:
			UE_LOG(LogTemp, Log, TEXT("BOAT SETUP STARTED"));
			break;
	}
	Init_VehicleAnim(VehicleData->Anim_Class.Get());
	ApplyCamoToVehicle(VehicleStartingData.StartingVehicleLoadout.VehicleCamo);
	SpawnComponent->Init_SpawnData(VehicleData->Vehicle_DisplayName, VehicleData->VehicleIcon.Get(), ESpawnType::Vehicle);
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
	VehicleCurrentState.GenericVehicleState.HornAudioComponent->Play(0.0f);
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &AVehicle_Base::StopHorn, 4.0f, false);
}

void AVehicle_Base::StopHorn()
{
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

UCameraComponent* AVehicle_Base::SpawnAndAttachCamera(FName SocketToAttach, USkeletalMeshComponent* MeshToAttachTo)
{
	UCameraComponent* Cam = NewObject<UCameraComponent>(this);
	Cam->SetupAttachment(MeshToAttachTo, SocketToAttach);
	Cam->RegisterComponent();
	return Cam;
}

#pragma region SeatManagement

bool AVehicle_Base::CycleThroughSeats(ACharacter_Base* Character)
{
	int32 TotalSeats = VehicleData->Seats.Num();
	int32& StartIndex = Character->GetCSI();
	int32 Offset = (TotalSeats == 1) ? 0 : 1;

	//try each seat exactly once, skipping the current seat
	for (; Offset < TotalSeats; ++Offset)
	{
		int32 CheckIndex = (StartIndex + Offset) % TotalSeats;		//wraps around the seat list circularly, so if you’re at the last seat, it loops back to seat 0.
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
	//move this to character?
	if (SeatData.SeatRole == E_SeatRole::DriverGunner || SeatData.SeatRole == E_SeatRole::Gunner)
	{
		const FVehicleWeaponSystem_Runtime& VWS = *VehicleWeaponLogicComponent->VehicleWeaponSystem.Find(Character->GetCSI());
		const FVehicleWeapon_Runtime& EquippedWeapon = VWS.Weapons[VWS.VehicleWeaponSystemState.EquippedWeaponState.CurrentWeaponIndex];
		if (EquippedWeapon.VehicleWeaponInstanceData.bHasSpecialCam)
		{
			TWeakObjectPtr<AActor> ViewTarget = nullptr;
			TWeakObjectPtr<UCameraComponent> Cam = VehicleCurrentState.SeatStates[Character->GetCSI()].ActiveCamera;
			switch (EquippedWeapon.VehicleWeaponInstanceData.WeaponCamBehavior.MountMethod)
			{
				case EVehicleWeaponCamMountMethod::VehicleMesh:
				case EVehicleWeaponCamMountMethod::WeaponMesh:
				case EVehicleWeaponCamMountMethod::MountedProjectile:
					ViewTarget = VehicleWeaponLogicComponent->GetCurrentViewTargetAtSeatIndex(Character->GetCSI());
					Character->UpdateViewTarget(ViewTarget, Cam);
					break;
			}
		}
		else
		{
			HandleViewMethod_Default(Character, SeatData);
		}
	}
	else
	{
		HandleViewMethod_Default(Character, SeatData);
	}
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
			if (VehicleCurrentState.SeatStates[Character->GetCSI()].ActiveCamera)
			{

				Character->UpdateViewTarget(this, VehicleCurrentState.SeatStates[Character->GetCSI()].ActiveCamera);
			}
			else
			{
				Character->UpdateViewTarget(this, VehicleCurrentState.SeatStates[Character->GetCSI()].DefaultCamera);
			}
			break;
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

	if (SeatData.ViewMethod == E_ViewMethod::Remote && VehicleCurrentState.SeatStates[SeatIndex].ActiveCamera)
	{
		VehicleCurrentState.SeatStates[SeatIndex].ActiveCamera->SetActive(false);
	}

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
			GetWorld()->GetTimerManager().SetTimer(RotorUpdateTimer, this, &AVehicle_Base::UpdateRotorRPM, 0.016f, true);
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
	UE_LOG(LogTemp, Log, TEXT("ENTER VEHICLE"));
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

#pragma endregion

void AVehicle_Base::UpdateSeatActiveCamera(int32 SeatIndex, UCameraComponent* NewActiveCamera)
{
	VehicleCurrentState.SeatStates[SeatIndex].ActiveCamera = NewActiveCamera;
}

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

void AVehicle_Base::UpdateThrottle_GV(float InputValue)
{
	//if this doesnt work reference that 1 tutorial you did
	if (InputValue > 0)
	{
		ChaosVehicleMovement->SetThrottleInput(InputValue);
		UBS2FunctionLibrary::GetHUDSubsystem(this)->UpdateSpeedHUD_Vehicle(GetVelocity().Size());		//block behind updatehud check?
	}
	else if (InputValue < 0)
	{
		ChaosVehicleMovement->SetThrottleInput(0.0f);
		ChaosVehicleMovement->SetBrakeInput(FMath::Abs(InputValue));
		UBS2FunctionLibrary::GetHUDSubsystem(this)->UpdateSpeedHUD_Vehicle(GetVelocity().Size());		//block behind updatehud check?
	}
	else if (InputValue == 0)
	{
		ChaosVehicleMovement->SetThrottleInput(0);
		ChaosVehicleMovement->SetBrakeInput(0);
		
		GetWorld()->GetTimerManager().SetTimer(SpeedTimer, [this]()
		{
			UBS2FunctionLibrary::GetHUDSubsystem(this)->UpdateSpeedHUD_Vehicle(GetVelocity().Size());		//block behind updatehud check?
		}, 0.05f, true);
		if (GetVelocity().Size() <= 0.0f)
		{
			GetWorld()->GetTimerManager().ClearTimer(SpeedTimer);
		}
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
	if (VehicleCurrentState.SeatStates[SeatIndex].ActiveCamera)
	{
		UBS2FunctionLibrary::GetHUDSubsystem(this)->UpdateCompassHUD_Vehicle(VehicleCurrentState.SeatStates[SeatIndex].ActiveCamera->GetComponentRotation().Yaw);
	}
}

#pragma region HeliInput

void AVehicle_Base::UpdateThrottle_Heli(float InputValue)
{
	float& CurrentHoverVelocity = VehicleCurrentState.AircraftState.HelicopterState.CurrentHoverVelocity;
	const FHelicopterData& HeliData = VehicleData->Helicopter_Data;
	float NewTargetValue;
	FVector NewVelocity;
	if (InputValue != 0.0f)
	{
		NewTargetValue = InputValue * HeliData.MaxThrust;
		CurrentHoverVelocity = FMath::FInterpTo(CurrentHoverVelocity, NewTargetValue, GetWorld()->GetDeltaSeconds(), 2.0f);
		NewVelocity = (CurrentHoverVelocity * GetActorUpVector()) * (GetWorld()->GetDeltaSeconds() * 100.0f);
	}
	else
	{
		CurrentHoverVelocity = FMath::FInterpTo(CurrentHoverVelocity, HeliData.ThrottlePower, GetWorld()->GetDeltaSeconds(), 4.0f);
		NewVelocity = (CurrentHoverVelocity * GetActorUpVector()) * (GetWorld()->GetDeltaSeconds() * 100.0f);
	}
	VehicleMeshComponent->SetAllPhysicsLinearVelocity(NewVelocity, true);

	//FVector ThrustForce = GetActorUpVector() * CurrentHoverVelocity * VehicleMeshComponent->GetMass();
	//VehicleMeshComponent->AddForce(ThrustForce);
}

void AVehicle_Base::UpdatePitch_Heli(float InputValue)
{
	float& CurrentPitchSpeed = VehicleCurrentState.AircraftState.HelicopterState.CurrentPitchSpeed;
	const FHelicopterData& HeliData = VehicleData->Helicopter_Data;

	CurrentPitchSpeed = FMath::FInterpTo(CurrentPitchSpeed, InputValue * HeliData.MaxPitchSpeed, GetWorld()->GetDeltaSeconds(), 1.5f);
	float PitchThisFrame = CurrentPitchSpeed * GetWorld()->GetDeltaSeconds();
	FRotator DeltaRotation = FRotator(PitchThisFrame, 0.0f, 0.0f);
	AddActorLocalRotation(DeltaRotation, false, nullptr, ETeleportType::TeleportPhysics);

	UBS2FunctionLibrary::GetHUDSubsystem(this)->UpdateAltitudeHUD_Vehicle(GetActorRotation().Pitch);
}

void AVehicle_Base::UpdateYaw_Heli(float InputValue)
{
	float& CurrentYawSpeed = VehicleCurrentState.AircraftState.HelicopterState.CurrentYawSpeed;
	const FHelicopterData& HeliData = VehicleData->Helicopter_Data;

	CurrentYawSpeed = FMath::FInterpTo(CurrentYawSpeed, InputValue * HeliData.MaxYawSpeed, GetWorld()->GetDeltaSeconds(), 2.0f);
	float YawThisFrame = CurrentYawSpeed * GetWorld()->GetDeltaSeconds();
	AddActorLocalRotation(FRotator(0.0f, YawThisFrame, 0.0f), false, nullptr, ETeleportType::TeleportPhysics);
}

void AVehicle_Base::UpdateRoll_Heli(float InputValue)
{
	float& CurrentRollSpeed = VehicleCurrentState.AircraftState.HelicopterState.CurrentRollSpeed;
	const FHelicopterData& HeliData = VehicleData->Helicopter_Data;
	CurrentRollSpeed = FMath::FInterpTo(CurrentRollSpeed, InputValue * HeliData.MaxRollSpeed, GetWorld()->GetDeltaSeconds(), 1.5f);
	float RollThisFrame = CurrentRollSpeed * GetWorld()->GetDeltaSeconds();
	AddActorLocalRotation(FRotator(0.0f, 0.0f, RollThisFrame), false, nullptr, ETeleportType::TeleportPhysics);
}

#pragma endregion

#pragma region JetInput

void AVehicle_Base::UpdateThrottle_Jet(float InputValue)
{
	const FJetData& JetData = VehicleData->Jet_Data;
	const FThrottleFlightModel& ThrottleConfig = JetData.FlightModel.Throttle;
	FJetState& JetState = VehicleCurrentState.AircraftState.JetState;

	// 1. Landing Gear Power Limiter
	// Reduces max thrust when gear is down to simulate drag/safety
	//float GearModifier = VehicleCurrentState. ? ThrottleConfig.GearDownSpeedScalar : 1.0f;
	float TargetInput = InputValue;

	// 2. Actuator Spool (The Timeline Replacement)
	JetState.CurrentThrottle = FMath::FInterpTo(JetState.CurrentThrottle, TargetInput, GetWorld()->GetDeltaSeconds(), ThrottleConfig.ThrottleSpeed);

	// 3. Execute Physics Model
	switch (JetData.FlightModelType)
	{
		case EFlightModelType::Kinematic:
		{
			// Direct Velocity: ThrustStrength acts as Max Speed (cm/s)
			FVector TargetVelocity = GetActorForwardVector() * (JetState.CurrentThrottle * ThrottleConfig.ThrustStrength);
			VehicleMeshComponent->SetAllPhysicsLinearVelocity(TargetVelocity, false);
			break;
		}
		case EFlightModelType::Dynamic:
		{
			// Force-Based: Apply thrust to the Center of Mass
			FVector ThrustForce = GetActorForwardVector() * (JetState.CurrentThrottle * ThrottleConfig.ThrustStrength);

			// bAccelChange = true ignores mass for easier tuning
			VehicleMeshComponent->AddForce(ThrustForce, NAME_None, true);
			break;
		}
		case EFlightModelType::LinearChaos:
		{
			ChaosVehicleMovement->SetThrottleInput(JetState.CurrentThrottle);
			break;
		}
	}
}

void AVehicle_Base::UpdatePitch_Jet(float InputValue)
{
	const FJetData& JetData = VehicleData->Jet_Data;
	const FPitchFlightModel& PitchConfig = JetData.FlightModel.Pitch;
	FJetState& JetState = VehicleCurrentState.AircraftState.JetState;

	// 1. Calculate Speed Sensitivity
	float SpeedInKMH = GetVelocity().Size() * 0.036f;
	float Sensitivity = 1.0f;
	if (PitchConfig.PitchSensitivityCurve)
	{
		Sensitivity = PitchConfig.PitchSensitivityCurve->GetFloatValue(SpeedInKMH);
	}

	// 2. Actuator (Elevator Movement)
	// Stiffens controls based on speed sensitivity
	float DynamicLimit = PitchConfig.InputLimit * Sensitivity;
	float TargetElevatorPos = FMath::Clamp(InputValue, -DynamicLimit, DynamicLimit);

	JetState.CurrentElevatorPitch = FMath::FInterpTo(JetState.CurrentElevatorPitch,TargetElevatorPos, GetWorld()->GetDeltaSeconds(), PitchConfig.PitchSpeed);

	// 3. Execute Physics Model
	switch (JetData.FlightModelType)
	{
		case EFlightModelType::Kinematic:
		{
			// Arcade: Rotate by Degrees Per Second
			float PitchAmount = JetState.CurrentElevatorPitch * PitchConfig.PitchStrength;
			float DeltaRot = PitchAmount * GetWorld()->GetDeltaSeconds();

			AddActorLocalRotation(FRotator(DeltaRot, 0.0f, 0.0f), false, nullptr, ETeleportType::TeleportPhysics);
			break;
		}

		case EFlightModelType::Dynamic:
		{
			FVector TorqueVector = GetActorRightVector() * (JetState.CurrentElevatorPitch * PitchConfig.PitchStrength);
			VehicleMeshComponent->AddTorqueInDegrees(TorqueVector, NAME_None, true);
			break;
		}

		case EFlightModelType::LinearChaos:
		{
			ChaosVehicleMovement->SetPitchInput(JetState.CurrentElevatorPitch);
			break;
		}
	}
}

void AVehicle_Base::UpdateRoll_Jet(float InputValue)
{
	const FJetData& JetData = VehicleData->Jet_Data;
	const FRollFlightModel& RollConfig = JetData.FlightModel.Roll;
	FJetState& JetState = VehicleCurrentState.AircraftState.JetState;

	JetState.CurrentAileronRoll = FMath::FInterpTo(JetState.CurrentAileronRoll, InputValue, GetWorld()->GetDeltaSeconds(), JetData.FlightModel.Roll.RollSpeed);

	switch (JetData.FlightModelType)
	{
		case EFlightModelType::Kinematic:
		{
			float RollDegrees = JetState.CurrentAileronRoll * RollConfig.RollStrength * GetWorld()->GetDeltaSeconds();
			AddActorLocalRotation(FRotator(0.0f, 0.0f, RollDegrees), false, nullptr, ETeleportType::TeleportPhysics);
			break;
		}
		case EFlightModelType::LinearChaos:
			ChaosVehicleMovement->SetRollInput(JetState.CurrentAileronRoll);
			break;
	}
}

void AVehicle_Base::UpdateYaw_Jet(float InputValue)
{
	const FJetData& JetData = VehicleData->Jet_Data;
	const FYawFlightModel& YawConfig = JetData.FlightModel.Yaw;
	FJetState& JetState = VehicleCurrentState.AircraftState.JetState;

	JetState.CurrentRudderYaw = JetState.CurrentRudderYaw = FMath::FInterpTo(JetState.CurrentRudderYaw, InputValue, GetWorld()->GetDeltaSeconds(),YawConfig.YawSpeed);

	switch (JetData.FlightModelType)
	{
		case EFlightModelType::Kinematic:
		{
			float YawDegrees = JetState.CurrentRudderYaw * YawConfig.YawStrength * GetWorld()->GetDeltaSeconds();
			AddActorLocalRotation(FRotator(0.0f, YawDegrees, 0.0f), false, nullptr, ETeleportType::TeleportPhysics);
			break;
		}
		case EFlightModelType::LinearChaos:
			ChaosVehicleMovement->SetYawInput(JetState.CurrentRudderYaw);
			break;
	}
}

#pragma endregion

void AVehicle_Base::Input_HandleThrottle(float ThrottleValue)
{
	const E_MovementType& MovementType = VehicleData->Movement_Type;
	switch (MovementType)
	{
		case E_MovementType::GroundVehicle:
			UpdateThrottle_GV(ThrottleValue);
			break;
		case E_MovementType::Helicopter:
			UpdateThrottle_Heli(ThrottleValue);
			break;
		case E_MovementType::Jet:
			UpdateThrottle_Jet(ThrottleValue);
			break;
	}
}

void AVehicle_Base::Input_ReleaseThrottle()
{
	const E_MovementType& MovementType = VehicleData->Movement_Type;
	switch (MovementType)
	{
	case E_MovementType::GroundVehicle:
		UpdateThrottle_GV(0);
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

UCameraComponent* AVehicle_Base::GetRemoteActiveCam(int32 SeatIndex)
{
	return VehicleCurrentState.SeatStates[SeatIndex].ActiveCamera;
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





