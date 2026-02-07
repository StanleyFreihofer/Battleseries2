// Fill out your copyright notice in the Description page of Project Settings.

#include "Character_Base.h"
#include "Vehicle_Base.h"
#include "Data/Runtime/VehicleTypes.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Data/Weapons/Data_Weapon.h"
#include "Core/Weapons/VehicleWeaponLogicComponent.h"
#include "Core/UI/VehicleHUDs/UW_HUD_Vehicle_Base.h"
#include "Core/PlayerController_Base.h"
#include "Utilities/HUDSubsystem.h"

// Sets default values
ACharacter_Base::ACharacter_Base()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	Camera->SetupAttachment(GetMesh(), FName("Camera"));
}

// Called when the game starts or when spawned
void ACharacter_Base::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ACharacter_Base::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (CharacterState.CharacterVehicleState.inVehicle)
	{
		//GetLocalPlayerHUDSystem()->UpdateCompassHUD_Vehicle();		//this or on steer and control turret 
		//GetLocalPlayerHUDSystem()->UpdateTurretLinesHUD_Vehicle();		//control turret
	}
}

// Called to bind functionality to input
void ACharacter_Base::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	InputComponent_Player = PlayerInputComponent;
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (DefaultIMCSoft.IsValid())
	{
		DefaultIMC = DefaultIMCSoft.Get();
	}
	else
	{
		DefaultIMC = DefaultIMCSoft.LoadSynchronous(); // Force-load if not already in memory
	}

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Add mapping context
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
			{
				Subsystem->AddMappingContext(DefaultIMC, 0);
			}
		}
	}
}

void ACharacter_Base::ManageinVehicleStatus(AVehicle_Base* Vehicle, bool In_Vehicle)
{
	CharacterState.CharacterVehicleState.inVehicle = In_Vehicle;
	CharacterState.CharacterVehicleState.CurrentVehicle = Vehicle;
	if (CharacterState.CharacterVehicleState.inVehicle)
	{
		CharacterEnterVehicle();
	}
	else
	{
		CharacterExitVehicle();
	}
}

void ACharacter_Base::UpdateSeatList(TArray<ACharacter_Base*> Characters)
{
	TArray<FSeatState> Seats = CharacterState.CharacterVehicleState.CurrentVehicle->VehicleCurrentState.SeatStates;
	for (int32 SI = 0; SI < Seats.Num(); SI++)
	{
		for (ACharacter_Base* Character : Characters)
		{
			if (Character->CharacterState.CharacterVehicleState.CSI == SI)
			{
				//do something UI here (show seat as occupied, that characters name, etc)
				continue;
			}
		}
	}
}

void ACharacter_Base::CharacterEnterVehicle()
{
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Vehicle, ECollisionResponse::ECR_Ignore);
	GetCharacterMovement()->SetMovementMode(MOVE_None);
	AttachToActor(CharacterState.CharacterVehicleState.CurrentVehicle, FAttachmentTransformRules::KeepRelativeTransform);
	Camera->bUsePawnControlRotation = false;
	Camera->SetRelativeRotation(FRotator());
	bUseControllerRotationYaw = false;
}

void ACharacter_Base::CharacterExitVehicle()
{
	if (CharacterState.CharacterVehicleState.CurrentVehicle)
	{
		DetachFromActor(FDetachmentTransformRules::KeepRelativeTransform);
	}
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Vehicle, ECollisionResponse::ECR_Block);
	Camera->bUsePawnControlRotation = true;
	bUseControllerRotationYaw = true;
}

void ACharacter_Base::CharacterEnterSeat(const FCharacterSeatContext& SeatContext)
{
	SetActorRelativeTransform(SeatContext.SeatTransform);
	UpdateCharacterStance(SeatContext.SeatStance);
	ManageIMC(nullptr, SeatContext.InputMappingContext, 1);
	UpdateCharacterMeshVisibility(SeatContext.bIsCharacterVisible);
	if (SeatContext.SeatHUD)
	{
		UpdateVehicleHUD(SeatContext.SeatHUD);
	}

	//sync vehicle states for hud
	switch (CharacterState.CharacterVehicleState.CurrentVehicle->VehicleData->Seats[CharacterState.CharacterVehicleState.CSI].SeatRole)
	{
		case E_SeatRole::Driver:
			GetLocalPlayerHUDSystem()->UpdateSpeedHUD_Vehicle(GetCurrentVehicle()->GetVelocity().Size());
			break;
		case E_SeatRole::Gunner:
			break;
		case E_SeatRole::DriverGunner:
			GetLocalPlayerHUDSystem()->UpdateSpeedHUD_Vehicle(GetCurrentVehicle()->GetVelocity().Size());

			//turrets/heading
			GetLocalPlayerHUDSystem()->UpdateCompassHUD_Vehicle(GetCurrentVehicle()->VehicleCurrentState.SeatStates[GetCSI()].ActiveCamera->GetComponentRotation().Yaw);
			const int32& CTI = GetCurrentVehicle()->VehicleData->Seats[GetCSI()].AvailableItems.ControlledTurretIndexes[0];

			GetLocalPlayerHUDSystem()->HandleTurretRotationUpdate(GetCurrentVehicle()->VehicleCurrentState.SeatStates[GetCSI()].ActiveCamera->GetComponentRotation().Yaw);
			
			GetLocalPlayerHUDSystem()->HandleTurretPitchUpdate
			(
				GetCurrentVehicle()->VehicleData->Turrets[CTI].TurretPitch.TurretMinMax.GetLowerBoundValue(),
				GetCurrentVehicle()->VehicleData->Turrets[CTI].TurretPitch.TurretMinMax.GetUpperBoundValue(),
				GetCurrentVehicle()->VehicleWeaponLogicComponent->TurretStates[CTI].CurrentTurretPitch
			);
			break;
	}
}

void ACharacter_Base::CharacterExitSeat(const FCharacterSeatContext& SeatContext)
{
	UpdateVehicleHUD(nullptr);
	ManageIMC(SeatContext.InputMappingContext, nullptr, 0);
}

void ACharacter_Base::UpdateSeatIndexes(int32 NewLSI, int32 NewCSI, int32 NewNSI)
{
	CharacterState.CharacterVehicleState.LSI = NewLSI;
	CharacterState.CharacterVehicleState.CSI = NewCSI;
	CharacterState.CharacterVehicleState.NSI = NewNSI;
}

void ACharacter_Base::UpdateCharacterStance(ECharacterCurrentStance NewStance)
{
	CharacterState.CurrentStance = NewStance;
}

void ACharacter_Base::UpdateViewTarget(TWeakObjectPtr<AActor> NewViewTarget, TWeakObjectPtr<UCameraComponent> CameraComponent)
{
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->SetViewTarget(NewViewTarget.Get());
		CameraComponent->SetActive(true);
	}
}

void ACharacter_Base::BindInputAction(class UInputComponent* PlayerInputComponent, const UInputAction* IA, ETriggerEvent InputType, FName InputFunctionName)
{
	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (UFunction* Function = FindFunction(InputFunctionName))
	{
		FEnhancedInputActionHandlerDynamicSignature Delegate;
		Delegate.BindUFunction(this, InputFunctionName);
		EnhancedInput->BindAction(IA, InputType, this, InputFunctionName);
	}
}

void ACharacter_Base::HandleFireCompleted()
{
	if (CharacterState.CharacterVehicleState.inVehicle)
	{
		OnFireReleased_Vehicle.Broadcast(CharacterState.CharacterVehicleState.CSI);
	}
}

void ACharacter_Base::ManageIMC(UInputMappingContext* IMC_ToRemove, UInputMappingContext* IMC_ToAdd, int32 IMCAddPriority)
{
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
		if (IMC_ToRemove)
		{
			Subsystem->RemoveMappingContext(IMC_ToRemove);
		}
		if (IMC_ToAdd)
		{
			Subsystem->AddMappingContext(IMC_ToAdd, IMCAddPriority);
		}
	}
}

void ACharacter_Base::HideCharacterMesh()
{
	GetMesh()->SetHiddenInGame(true);
	GetMesh()->SetComponentTickEnabled(false); 
	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickMontagesWhenNotRendered;
}

void ACharacter_Base::ShowCharacterMesh()
{
	GetMesh()->SetHiddenInGame(false);
	GetMesh()->SetComponentTickEnabled(true);
	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	GetMesh()->RefreshBoneTransforms();
	GetMesh()->UpdateComponentToWorld();
}

void ACharacter_Base::UpdateCharacterMeshVisibility(bool ShowMesh)
{
	if (ShowMesh)
	{
		ShowCharacterMesh();
	}
	else
	{
		HideCharacterMesh();
	}
}

void ACharacter_Base::UpdateRangefinder_WindowedVehicle()
{
	if (CharacterState.CharacterVehicleState.inVehicle && !CharacterState.CharacterVehicleState.isFreeLooking)
	{
		const FSeatData& OccupiedSeatData = CharacterState.CharacterVehicleState.CurrentVehicle->VehicleData->Seats[CharacterState.CharacterVehicleState.CSI];
		if (OccupiedSeatData.ViewMethod == E_ViewMethod::Windowed)
		{
			if (OccupiedSeatData.SeatRole == E_SeatRole::DriverGunner || OccupiedSeatData.SeatRole == E_SeatRole::Gunner)
			{
				UVehicleWeaponLogicComponent* VWLC = CharacterState.CharacterVehicleState.CurrentVehicle->VehicleWeaponLogicComponent;
				FVehicleWeaponSystem_Runtime* SeatWeaponSystem = VWLC->VehicleWeaponSystem.Find(CharacterState.CharacterVehicleState.CSI);
				if (!SeatWeaponSystem->Weapons[SeatWeaponSystem->VehicleWeaponSystemState.EquippedWeaponState.CurrentWeaponIndex].VehicleWeaponInstanceData.bHasSpecialCam)
				{
					TArray<AActor*> Actors;
					Actors.Add(CharacterState.CharacterVehicleState.CurrentVehicle);
					VWLC->UpdateSeatRangefinder(CharacterState.CharacterVehicleState.CSI, Camera, Actors);
				}
			}
		}
	}
}

UHUDSubsystem* ACharacter_Base::GetLocalPlayerHUDSystem()
{
	APlayerController_Base* PC = Cast<APlayerController_Base>(GetController());
	if (IsLocallyControlled())
	{
		if (ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (UHUDSubsystem* HUDSub = LP->GetSubsystem<UHUDSubsystem>())
			{
				return HUDSub;
			}
		}
	}
	return nullptr;
}

AVehicle_Base* ACharacter_Base::GetCurrentVehicle()
{
	if (CharacterState.CharacterVehicleState.CurrentVehicle)
	{
		return CharacterState.CharacterVehicleState.CurrentVehicle;
	}
	else
	{
		return nullptr;
	}
}

int32& ACharacter_Base::GetCSI()
{
	return CharacterState.CharacterVehicleState.CSI;
}

void ACharacter_Base::UpdateVehicleHUD(TSubclassOf<UUserWidget> HUDClass)
{
	if (UHUDSubsystem* HUDSub = GetLocalPlayerHUDSystem())
	{
		if (!HUDSub->CurrentVehicleHUD && HUDClass)
		{
			HUDSub->SpawnVehicleSeatHUD(HUDClass);
		}
		else
		{
			if (!HUDClass && HUDSub->CurrentVehicleHUD)
			{
				HUDSub->CurrentVehicleHUD->RemoveFromParent();
				HUDSub->CurrentVehicleHUD = nullptr;
			}
		}
	}

}
