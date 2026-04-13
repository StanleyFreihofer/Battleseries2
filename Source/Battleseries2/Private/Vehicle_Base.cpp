// Fill out your copyright notice in the Description page of Project Settings.

#include "Vehicle_Base.h"
#include "GameFramework/PlayerController.h"			//need because we currently call setviewtarget
#include "Camera/CameraActor.h"
#include "Data/Vehicles/Data_Vehicle.h"				//need to access members
#include "Data/Vehicles/Data_Seat.h"				//need to access members
#include "Data/Data_VehicleAttachments.h"
#include "Data/Data_Optics.h"
#include "Data/Vehicles/VehicleDefaults.h"
#include "Utilities/ProjectilePoolSubsystem.h"
#include "Utilities/HelperFunctions_Vehicle.h"
#include "Core/Weapons/Projectiles/Projectile_Base.h"
#include "Core/Weapons/VehicleWeaponLogicComponent.h"
#include "Core/Vehicles/ChaosWheel_Base.h"
#include "Core/PlayerController_Base.h"
#include "Core/UI/VehicleHUDs/UW_HUD_Vehicle_Base.h"
#include "Save/PlayerSave_Loadout.h"
#include "Save/SaveSubsystem.h"
#include "Utilities/GameInstance_Base.h"
#include "Utilities/HUDSubsystem.h"
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
	//DataManager = GetWorld()->GetGameInstance()->GetSubsystem<UDataManagerSubsystem>();
	SaveSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<USaveSubsystem>();

	if (!VehicleStartingData.VehicleID.IsNone())
	{
		if (GetDataManager()->IsDataReady())
		{
			Init_VehicleData();
		}
		else
		{
			GetDataManager()->OnDataReady.AddDynamic(this, &AVehicle_Base::Init_VehicleData);
		}
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

//GROUND VEHICLE FUNCTIONS
void AVehicle_Base::Init_Wheels()
{
	const TArray <FChaosWheelSetup>& WheelData = VehicleData->GroundVehicle_Data.WheelData;
	const TArray <FBaseWheelData>& BaseWheelData = VehicleData->GroundVehicle_Data.BaseWheelData;

	ChaosVehicleMovement->WheelSetups.Empty();
	ChaosVehicleMovement->Wheels.Empty();

	ChaosVehicleMovement->WheelSetups.SetNum(WheelData.Num());
	for (int32 i = 0; i < WheelData.Num(); i++)
	{
		const FChaosWheelSetup& SourceData = WheelData[i];				//the data we are pulling from
		FChaosWheelSetup& Setup = ChaosVehicleMovement->WheelSetups[i];	//the properties we are applying to
		Setup.WheelClass = SourceData.WheelClass;
		
		if (SourceData.WheelClass && SourceData.WheelClass->IsChildOf(UChaosWheel_Base::StaticClass()))
		{
			//Setup.WheelClass = SourceData.WheelClass;
			UChaosWheel_Base* WheelDefault = Setup.WheelClass->GetDefaultObject<UChaosWheel_Base>();		//WHEEL CLASS MUST BE THIS TYPE
			WheelDefault->Init_Wheel(BaseWheelData[i]);
		}	
	
		Setup.BoneName = SourceData.BoneName;							//set bone name
		Setup.AdditionalOffset = SourceData.AdditionalOffset;			//set additional offset
	}
}

void AVehicle_Base::Init_GroundVehicle()
{
	VehicleMeshComponent->RecreatePhysicsState();
	ChaosVehicleMovement->UnregisterComponent();
	ChaosVehicleMovement->SetUpdatedComponent(VehicleMeshComponent);
	Init_Wheels();

	//Mechanical Setup
	ChaosVehicleMovement->EnableMechanicalSim(true);	//true by default, not in DTs
	ChaosVehicleMovement->EngineSetup = VehicleData->GroundVehicle_Data.EngineData;
	ChaosVehicleMovement->DifferentialSetup = VehicleData->GroundVehicle_Data.DifferentialData;
	ChaosVehicleMovement->TransmissionSetup = VehicleData->GroundVehicle_Data.TransmissionData;
	ChaosVehicleMovement->SteeringSetup = VehicleData->GroundVehicle_Data.SteeringData;
	
	//Vehicle Setup
	ChaosVehicleMovement->Mass = VehicleData->GroundVehicle_Data.Mass;
	ChaosVehicleMovement->CenterOfMassOverride = VehicleData->GroundVehicle_Data.Center_Of_Mass_Override;

	//Vehicle Input
	//Yaw Input Rate
	ChaosVehicleMovement->YawInputRate.RiseRate = VehicleData->GroundVehicle_Data.Yaw_Input_Rise_Rate;
	ChaosVehicleMovement->YawInputRate.FallRate = VehicleData->GroundVehicle_Data.Yaw_Input_Fall_Rate;

	//allows the vehicle/chaos vehicle to actual simulate physics properly
	HandleChaosMovement(true);
	ChaosVehicleMovement->RegisterComponent();
	ChaosVehicleMovement->RecreatePhysicsState();;
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

//HELICOPTER FUNCTIONS
void AVehicle_Base::Init_Helicopter()
{

}

//SEAT FUNCTIONS
void AVehicle_Base::Init_DefaultSeatRemoteCamera(int32 SeatIndex)
{
	//init default seat camera
	//every seat that is remote should do this
	//how does this work with ConfigureWeaponCam on VehicleWeaponLogicComp???????????????
	E_ViewMethod ViewMethod = VehicleData->Seats[SeatIndex].ViewMethod;
	FString CameraSocketString = FString::Printf(TEXT("SC_%02d"), SeatIndex);		//SeatCam_SeatIndex		[SC_00]
	FName CameraSocketName = FName(*CameraSocketString);
	UCameraComponent* NewCamera;
	switch (ViewMethod)
	{
		case E_ViewMethod::Remote:
			NewCamera = SpawnAndAttachCamera(CameraSocketName, VehicleMeshComponent);
			VehicleCurrentState.SeatStates[SeatIndex].DefaultCamera = NewCamera;
			UpdateSeatActiveCamera(SeatIndex, NewCamera);
			//optic
			if (VehicleStartingData.StartingVehicleLoadout.SeatLoadout.Find(SeatIndex) && !VehicleStartingData.StartingVehicleLoadout.SeatLoadout.Find(SeatIndex)->Optic.IsNone())
			{
				VehicleCurrentState.SeatStates[SeatIndex].OpticState.CurrentAvailableOptics.Init(VehicleStartingData.StartingVehicleLoadout.SeatLoadout.Find(SeatIndex)->Optic, 1);
			}
			else
			{
				VehicleCurrentState.SeatStates[SeatIndex].OpticState.CurrentAvailableOptics.Init(VehicleData->Seats[SeatIndex].DefaultOptic, 1);
			}
			UpdateRemoteCamPP(SeatIndex, GetDataManager()->GetOpticDataRow(VehicleCurrentState.SeatStates[SeatIndex].OpticState.CurrentAvailableOptics[VehicleCurrentState.SeatStates[SeatIndex].OpticState.CurrentOpticIndex])->OpticPPSettings, 1.0f);
			break;
	}
}

void AVehicle_Base::Init_SeatHUDComp(int32& SeatIndex)
{
	UWidgetComponent* CockpitHUDComponent = NewObject<UWidgetComponent>(this);
	CockpitHUDComponent->RegisterComponent();
	CockpitHUDComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	CockpitHUDComponent->SetRelativeTransform(VehicleData->Seats[SeatIndex].DefaultCharacterContext.SeatHUDTransform);
	CockpitHUDComponent->SetWidgetClass(VehicleData->Seats[SeatIndex].DefaultCharacterContext.SeatHUD);
	CockpitHUDComponent->SetDrawSize(VehicleData->Seats[SeatIndex].DefaultCharacterContext.SeatHUDDrawSize);
	CockpitHUDComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CockpitHUDComponent->SetPivot(FVector2D(0.5f, 0.5f));
	CockpitHUDComponent->SetOwnerNoSee(false);
	VehicleCurrentState.SeatStates[SeatIndex].SeatHUDComponent = CockpitHUDComponent;

	//reticle quad
	UStaticMesh* DefaultPlane = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
	TWeakObjectPtr<UStaticMeshComponent> NewQuad = NewObject<UStaticMeshComponent>(this);
	NewQuad->RegisterComponent();
	NewQuad->AttachToComponent(CockpitHUDComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
	NewQuad->SetStaticMesh(DefaultPlane);
	NewQuad->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	NewQuad->SetCastShadow(false);
	NewQuad->SetReceivesDecals(false);
	NewQuad->SetRelativeRotation(FRotator(0.f, -90.f, 90.f));
	NewQuad->SetRelativeLocation(FVector(0.1f, 0.f, 0.f));

	UMaterialInterface* MasterMat = GetDataManager()->GetVehicleDefaults()->HUDMasterMaterial.LoadSynchronous();
	UMaterialInstanceDynamic* DynMat = NewQuad->CreateDynamicMaterialInstance(0, MasterMat);		//both creates and assigns

	FVehicleWeaponSystem_Runtime* VWS = VehicleWeaponLogicComponent->VehicleWeaponSystem.Find(SeatIndex);
	if (VWS)
	{
		VWS->VehicleWeaponSystemState.ReticleQuad = NewQuad;
	}
}

void AVehicle_Base::Init_Seats()
{
	VehicleCurrentState.SeatStates.SetNum(VehicleData->Seats.Num());
	for (int32 SI = 0; SI < VehicleData->Seats.Num(); ++SI)
	{
		const FSeatData& SeatInfo = VehicleData->Seats[SI];

		Init_DefaultSeatRemoteCamera(SI);	

		if (SeatInfo.DefaultCharacterContext.SeatHUD)
		{
			Init_SeatHUDComp(SI);
		}

		for (int32 i = 0; i < VehicleStartingData.OccupiedSeats.Num(); ++i)
		{
			if (VehicleStartingData.OccupiedSeats[i] == SI)
			{
				VehicleCurrentState.SeatStates[SI].isOccupied = true;
				break;
			}
		}
	}

	OnSeatsInitialized.Broadcast();		//here so we can guarantee ALL NEW SEATS ARE SETUP (race conditions suck)
	UE_LOG(LogTemp, Error, TEXT("[Vehicle_Base::Init_Seats] STEP 2B FINISHED (end of Init_Seats). old seats destroyed. new seats spawned in, VehicleID = %s"), *VehicleStartingData.VehicleID.ToString());
}

//GENERIC FUNCTIONS
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
	VehicleData = GetDataManager()->GetVehicleDataRow(VehicleStartingData.VehicleID);

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
	Init_VehicleAnim(VehicleData->Anim_Class.Get());
	VehicleWeaponLogicComponent->Init_VehicleWeaponSystem(VehicleStartingData.StartingVehicleLoadout.SeatLoadout);		//weapons and turrets
	Init_Seats();
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
			break;
		case E_MovementType::Jet:
			UE_LOG(LogTemp, Log, TEXT("JET SETUP STARTED"));
			break;
		case E_MovementType::Boat:
			UE_LOG(LogTemp, Log, TEXT("BOAT SETUP STARTED"));
			break;
	}
	ApplyCamoToVehicle(VehicleStartingData.StartingVehicleLoadout.VehicleCamo);

	SpawnComponent->Init_SpawnData(FText::FromName(VehicleData->Vehicle_DisplayName), VehicleData->VehicleIcon.Get(), ESpawnType::Vehicle);
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

void AVehicle_Base::ApplyLoadoutToSeat(int32 SeatIndex)		//this functions applies everything in the loadout (weapons, optics, upgrades, camos (if applicable), etc)
{
	//now only called by ApplyLoadoutToVehicle when enter main seat (driver/drivergunner) and is then applied TO EVERY SEAT
	switch (VehicleData->Seats[SeatIndex].SeatRole)
	{
		case E_SeatRole::DriverGunner:
		case E_SeatRole::Gunner:
			VehicleWeaponLogicComponent->HandleApplyWeaponsToSeat(SeatIndex);
			ApplyOpticToSeat(SeatIndex);
			break;
	}
}

void AVehicle_Base::ApplyOpticToSeat(int32 SeatIndex)
{
	USaveSubsystem* SaveSys = GetWorld()->GetGameInstance()->GetSubsystem<USaveSubsystem>();
	const FSavedSeatLoadout& SeatLoadoutSave = SaveSys->GetSeatLoadout(VehicleData->Vehicle_Type, SeatIndex);

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
		ApplyCamoToVehicle(SaveSubsystem->GetVehicleLoadout(VehicleData->Vehicle_Type).VehicleCamo);
		VehicleCurrentState.GenericVehicleState.LoadoutApplied = true;
	}
}

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

	const FVehicleAttachmentData& AttachmentData = *GetDataManager()->GetVehicleAttachmentDataRow(AttachmentID);
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

UCameraComponent* AVehicle_Base::SpawnAndAttachCamera(FName SocketToAttach, USkeletalMeshComponent* MeshToAttachTo)
{
	UCameraComponent* Cam = NewObject<UCameraComponent>(this);
	Cam->SetupAttachment(MeshToAttachTo, SocketToAttach);
	Cam->RegisterComponent();
	return Cam;
}

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
			Character->UpdateSeatIndexes(Character->CharacterState.CharacterVehicleState.CSI, CheckIndex, CheckIndex);
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
		const FVehicleWeaponSystem_Runtime& VWS = *VehicleWeaponLogicComponent->VehicleWeaponSystem.Find(Character->CharacterState.CharacterVehicleState.CSI);
		const FVehicleWeapon_Runtime& EquippedWeapon = VWS.Weapons[VWS.VehicleWeaponSystemState.EquippedWeaponState.CurrentWeaponIndex];
		if (EquippedWeapon.VehicleWeaponInstanceData.bHasSpecialCam)
		{
			TWeakObjectPtr<AActor> ViewTarget = nullptr;
			TWeakObjectPtr<UCameraComponent> Cam = VehicleCurrentState.SeatStates[Character->CharacterState.CharacterVehicleState.CSI].ActiveCamera;
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
			Character->UpdateViewTarget(Character, Character->Camera);
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

	if (SeatData.ViewMethod == E_ViewMethod::Remote)
	{
		if (VehicleCurrentState.SeatStates[SeatIndex].ActiveCamera)
		{
			VehicleCurrentState.SeatStates[SeatIndex].ActiveCamera->SetActive(false);
		}
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
	if (Character->IsLocallyControlled())
	{
		VehicleCurrentState.SeatStates[Character->GetCSI()].UpdateHUD = true;
	}
}

void AVehicle_Base::SetupDriver(ACharacter_Base* Character)
{
	//start engine
	UGameplayStatics::PlaySoundAtLocation(this, Cast<USoundBase>(VehicleData->GenericVehicleAudio.EngineStartupAudio), GetActorLocation(), FRotator::ZeroRotator, 1.0f, 1.0f, 0.0f);
	if (!VehicleCurrentState.GenericVehicleState.EngineAudioComponent)
	{
		//setup engine audio (do in factory instead?)
		VehicleCurrentState.GenericVehicleState.EngineAudioComponent = NewObject<UAudioComponent>(this, UAudioComponent::StaticClass());
		VehicleCurrentState.GenericVehicleState.EngineAudioComponent->RegisterComponent();
		VehicleCurrentState.GenericVehicleState.EngineAudioComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		VehicleCurrentState.GenericVehicleState.EngineAudioComponent->SetActive(true);
	}
	//update engine audio (do sound cue on a timer instead?)
	VehicleCurrentState.GenericVehicleState.EngineAudioComponent->SetSound(Cast<USoundBase>(VehicleData->GenericVehicleAudio.EngineAudio));
	GetWorldTimerManager().SetTimer(TimerHandle_AudioUpdate_Engine, this, &AVehicle_Base::UpdateEngineAudio, 0.05f, true);
	VehicleCurrentState.GenericVehicleState.EngineAudioComponent->Play();
	//engine/whatever else start up audio
}

void AVehicle_Base::DropDriver()
{
	//shutdown engine
	VehicleCurrentState.GenericVehicleState.EngineAudioComponent->Stop();
	GetWorldTimerManager().ClearTimer(TimerHandle_AudioUpdate_Engine);
	UGameplayStatics::PlaySoundAtLocation(this, Cast<USoundBase>(VehicleData->GenericVehicleAudio.EngineShutdownAudio), GetActorLocation(), FRotator::ZeroRotator, 1.0f, 1.0f, 0.0f);
}

void AVehicle_Base::SetupGunner(ACharacter_Base* Character)
{
	//called once on enter of a gunner seat, seat hud should already be on viewport
	const FSeatData& SeatData = VehicleData->Seats[Character->GetCSI()];
	if (SeatData.ViewMethod == E_ViewMethod::Windowed)
	{
		VehicleWeaponLogicComponent->WindowedRangefinder.AddDynamic(Character, &ACharacter_Base::UpdateRangefinder_WindowedVehicle);
	}

	VehicleWeaponLogicComponent->SelectWeapon(Character->GetCSI(), 0);
}

void AVehicle_Base::DropGunner(TWeakObjectPtr<ACharacter_Base> Character, int32& SeatIndex)
{
	VehicleWeaponLogicComponent->WindowedRangefinder.RemoveDynamic(Character.Get(), &ACharacter_Base::UpdateRangefinder_WindowedVehicle);
	VehicleWeaponLogicComponent->UnequipWeapon(SeatIndex, VehicleWeaponLogicComponent->GetCWIForSeat(SeatIndex), VehicleWeaponLogicComponent->GetEquippedWeaponInSeat(SeatIndex).VehicleWeaponState.BaseWeaponRuntimeData.WeaponState.isFiring);
}

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

void AVehicle_Base::UpdateSeatActiveCamera(int32 SeatIndex, UCameraComponent* NewActiveCamera)
{
	VehicleCurrentState.SeatStates[SeatIndex].ActiveCamera = NewActiveCamera;
}

void AVehicle_Base::UpdateRemoteCamPP(int32 SeatIndex, FPostProcessSettings PPSettings, float BlendWeight)
{
	GetRemoteActiveCam(SeatIndex)->PostProcessSettings = PPSettings;
	GetRemoteActiveCam(SeatIndex)->PostProcessBlendWeight = BlendWeight;
}

void AVehicle_Base::UpdateEngineAudio()
{
	switch (VehicleData->Movement_Type)
	{
		case E_MovementType::GroundVehicle:
			VehicleCurrentState.GenericVehicleState.EngineAudioComponent->SetFloatParameter(FName("RPM"), FMath::Abs(ChaosVehicleMovement->GetEngineRotationSpeed()));
			VehicleCurrentState.GenericVehicleState.EngineAudioComponent->SetFloatParameter(FName("Speed"), FMath::Abs(ChaosVehicleMovement->GetForwardSpeed() * 0.036f));
			break;
	}
}

void AVehicle_Base::ApplyThrottle_GV(float InputValue, int32 SeatIndex)
{
	//if this doesnt work reference that 1 tutorial you did
	if (InputValue > 0)
	{
		ChaosVehicleMovement->SetThrottleInput(InputValue);
		GetHUDSystem()->UpdateSpeedHUD_Vehicle(GetVelocity().Size());
	}
	else if (InputValue < 0)
	{
		ChaosVehicleMovement->SetBrakeInput(FMath::Abs(InputValue));
		GetHUDSystem()->UpdateSpeedHUD_Vehicle(GetVelocity().Size());
	}
	else if (InputValue == 0)
	{
		ChaosVehicleMovement->SetThrottleInput(0);
		ChaosVehicleMovement->SetBrakeInput(0);
		
		GetWorld()->GetTimerManager().SetTimer(SpeedTimer, [this]()
		{
			GetHUDSystem()->UpdateSpeedHUD_Vehicle(GetVelocity().Size());
		}, 0.05f, true);
		if (GetVelocity().Size() <= 0.0f)
		{
			GetWorld()->GetTimerManager().ClearTimer(SpeedTimer);
		}
	}
}

void AVehicle_Base::ApplySteering_GV(float SteeringValue, int32 SeatIndex)
{
	if (VehicleData->GroundVehicle_Data.canIdleTurn)
	{
		ChaosVehicleMovement->SetYawInput(SteeringValue);
	}
	else
	{
		ChaosVehicleMovement->SetSteeringInput(SteeringValue);
	}
	if (VehicleCurrentState.SeatStates[SeatIndex].ActiveCamera)
	{
		GetHUDSystem()->UpdateCompassHUD_Vehicle(VehicleCurrentState.SeatStates[SeatIndex].ActiveCamera->GetComponentRotation().Yaw);
	}
}

void AVehicle_Base::ToggleOptic(int32 SeatIndex)
{
	FOpticState& OpticState = VehicleCurrentState.SeatStates[SeatIndex].OpticState;
	int32 PreviousOpticIndex = OpticState.CurrentOpticIndex;
	OpticState.CurrentOpticIndex = (OpticState.CurrentOpticIndex + 1) % OpticState.CurrentAvailableOptics.Num();
	if (PreviousOpticIndex != OpticState.CurrentOpticIndex)
	{
		const FOpticData& CurrentOpticData = *GetDataManager()->GetOpticDataRow(OpticState.CurrentAvailableOptics[OpticState.CurrentOpticIndex]);
		const FOpticData& PreviousOpticData = *GetDataManager()->GetOpticDataRow(OpticState.CurrentAvailableOptics[PreviousOpticIndex]);
		//HandleTogglePPOptic
		if (CurrentOpticData.OpticPPSettings.WeightedBlendables.Array.Num() > 0)
		{

			UpdateRemoteCamPP(SeatIndex, GetDataManager()->GetOpticDataRow(OpticState.CurrentAvailableOptics[OpticState.CurrentOpticIndex])->OpticPPSettings, 1.0f);
			if (OpticState.isOn)
			{
				//what to do if optic was already on
			}
			else
			{
				TurnOnPPOptic(SeatIndex);
			}
		}
		else
		{
			if (OpticState.isOn)
			{
				TurnOffPPOptic(SeatIndex, PreviousOpticIndex);
			}
		}

		ToggleMagnificationOptic(SeatIndex, CurrentOpticData.ZoomMagnification);
	}
}

void AVehicle_Base::TurnOnPPOptic(int32 SeatIndex)
{
	FOpticState& OpticState = VehicleCurrentState.SeatStates[SeatIndex].OpticState;
	const FOpticData& CurrentOpticData = *GetDataManager()->GetOpticDataRow(OpticState.CurrentAvailableOptics[OpticState.CurrentOpticIndex]);
	OpticState.isOn = true;
	UGameplayStatics::PlaySound2D(GetWorld(), CurrentOpticData.PowerOnSound);
	if (VehicleCurrentState.SeatStates[SeatIndex].UpdateHUD)
	{
		GetHUDSystem()->UpdaticOpticNameHUD_Vehicle(CurrentOpticData.OpticDisplayNameAbrev);
		if (CurrentOpticData.InverseUIColor.A > 0)
		{
			GetHUDSystem()->UpdateVehicleHUD_Color(CurrentOpticData.InverseUIColor);
		}
	}
}

void AVehicle_Base::TurnOffPPOptic(int32 SeatIndex, int32 PreviousOpticIndex)
{
	FOpticState& OpticState = VehicleCurrentState.SeatStates[SeatIndex].OpticState;
	const FOpticData& CurrentOpticData = *GetDataManager()->GetOpticDataRow(OpticState.CurrentAvailableOptics[OpticState.CurrentOpticIndex]);
	const FOpticData& PreviousOpticData = *GetDataManager()->GetOpticDataRow(OpticState.CurrentAvailableOptics[PreviousOpticIndex]);
	UpdateRemoteCamPP(SeatIndex, FPostProcessSettings(), 0.0f);
	OpticState.isOn = false;
	UGameplayStatics::PlaySound2D(GetWorld(), PreviousOpticData.PowerOffSound);
	if (VehicleCurrentState.SeatStates[SeatIndex].UpdateHUD)
	{
		GetHUDSystem()->UpdaticOpticNameHUD_Vehicle(CurrentOpticData.OpticDisplayNameAbrev);
		if (PreviousOpticData.InverseUIColor.A > 0)
		{

		}
		if (CurrentOpticData.InverseUIColor.A == 0)
		{
			GetHUDSystem()->UpdateVehicleHUD_Color(FLinearColor(0.0f, 1.0f, 0.036889f, 1.0f));		//HARD CODED FOR NOW, CHANGE TO BE TAKEN FROM HUD/UI SETTINGS
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
			const FOpticData& CurrentOpticData = *GetDataManager()->GetOpticDataRow(OpticState.CurrentAvailableOptics[OpticState.CurrentOpticIndex]);
			CurrentFOV = NewFOV;
			OpticState.isMagnified = true;
			UGameplayStatics::PlaySound2D(GetWorld(), CurrentOpticData.PowerOnSound);
			if (VehicleCurrentState.SeatStates[SeatIndex].UpdateHUD)
			{
				GetHUDSystem()->UpdateOpticMagnificationHUD_Vehicle(ZoomMagnification);
			}
		}
	}
	//unzoom optic
	else if (OpticState.isMagnified)
	{
		//WORK ON THIS
		if (!OpticState.isOn)
		{
			const FOpticData& CurrentOpticData = *GetDataManager()->GetOpticDataRow(OpticState.CurrentAvailableOptics[OpticState.CurrentOpticIndex]);
			CurrentFOV = 90.0f;
			OpticState.isMagnified = false;
			UGameplayStatics::PlaySound2D(GetWorld(), CurrentOpticData.PowerOnSound);
			if (VehicleCurrentState.SeatStates[SeatIndex].UpdateHUD)
			{
				GetHUDSystem()->UpdateOpticMagnificationHUD_Vehicle(ZoomMagnification);
			}
		}
	}
}

void AVehicle_Base::HandleThrottle(float ThrottleValue, int32 SeatIndex)
{
	E_MovementType VehicleType = VehicleData->Movement_Type;
	switch (VehicleType)
	{
		case E_MovementType::GroundVehicle:
			ApplyThrottle_GV(ThrottleValue, SeatIndex);
			break;
	}
}

void AVehicle_Base::ReleaseThrottle(int32 SeatIndex)
{
	E_MovementType VehicleType = VehicleData->Movement_Type;
	switch (VehicleType)
	{
		case E_MovementType::GroundVehicle:
			ApplyThrottle_GV(0, SeatIndex);
			break;
	}
}

int32 AVehicle_Base::GetControlledTurret(int32 SeatIndex)
{
	return VehicleData->Seats[SeatIndex].AvailableItems.ControlledTurretIndexes[0];
}

UCameraComponent* AVehicle_Base::GetRemoteActiveCam(int32 SeatIndex)
{
	return VehicleCurrentState.SeatStates[SeatIndex].ActiveCamera;
}

TWeakObjectPtr<UHUDSubsystem> AVehicle_Base::GetHUDSystem()
{
	TWeakObjectPtr<ULocalPlayer> LP = GetWorld()->GetFirstLocalPlayerFromController();
	if (LP.Get())
	{
		TWeakObjectPtr<UHUDSubsystem> HUDSub = LP->GetSubsystem<UHUDSubsystem>();
		return HUDSub;
	}
	return nullptr;
}

UDataManagerSubsystem* AVehicle_Base::GetDataManager()
{
	return GetGameInstance()->GetSubsystem<UDataManagerSubsystem>();
}

USkeletalMeshComponent* AVehicle_Base::GetVehicleMesh() const
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

bool AVehicle_Base::GetIfCanLockOn_Implementation(const TArray<ETargetingCategory>& TargetingCategories, EHomingCapability HomingCapability)
{
	switch (HomingCapability)
	{
		case EHomingCapability::NoHoming:
			return false;
		case EHomingCapability::WireGuided:
			//if this vehicle is lazed
			break;
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





