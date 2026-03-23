// Fill out your copyright notice in the Description page of Project Settings.

#include "Character_Base.h"
#include "Vehicle_Base.h"
#include "Data/Runtime/VehicleTypes.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "Camera/CameraActor.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedPlayerInput.h"
#include "Data/Weapons/Data_Weapon.h"
#include "Core/Weapons/VehicleWeaponLogicComponent.h"
#include "Core/UI/VehicleHUDs/UW_HUD_Vehicle_Base.h"
#include "Core/PlayerController_Base.h"
#include "Utilities/HUDSubsystem.h"
#include "Kismet/KismetMathLibrary.h"


// Sets default values
ACharacter_Base::ACharacter_Base()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh(), FName("Camera"));
	Camera->SetupAttachment(CameraBoom, FName("SpringEndpoint"));
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

void ACharacter_Base::Input_Look(FVector2D InputAxisValue)
{
	if (!CharacterState.CharacterVehicleState.inVehicle)
	{
		AddControllerYawInput(InputAxisValue.X);
		AddControllerPitchInput(InputAxisValue.Y);
	}
	else
	{
		switch (GetCurrentVehicle()->VehicleData->Seats[GetCSI()].DefaultCharacterContext.CharacterRotationMethod)
		{
			case EControlRotationMethod::Freelook:
				Freelook(InputAxisValue);
				break;
		}
	}

}

void ACharacter_Base::Input_Move(FVector2D InputAxisValue)
{
	if (!CharacterState.CharacterVehicleState.inVehicle)
	{
		AddMovementInput(FVector(GetActorForwardVector()), InputAxisValue.Y);
		AddMovementInput(FVector(GetActorRightVector()), InputAxisValue.X);
	}
}

void ACharacter_Base::Input_SwitchWeapon_Vehicle()
{
	bool bSwitched = GetCurrentVehicle()->VehicleWeaponLogicComponent->SwitchWeapon(GetCSI());
	if (!bSwitched)
	{
		return;
	}
	const FSeatData& SeatData = GetCurrentVehicle()->VehicleData->Seats[GetCSI()];
	GetCurrentVehicle()->HandleViewMethod(this, SeatData);
}

void ACharacter_Base::Freelook(FVector2D InputAxisValue)
{
	float NewYaw = CharacterState.CurrentHeadDelta.X + InputAxisValue.X;
	float NewPitch = CharacterState.CurrentHeadDelta.Y + InputAxisValue.Y;
	NewPitch = FMath::Clamp(NewPitch, -80.0f, 80.0f);
	CharacterState.CurrentHeadDelta = FVector2D(NewYaw, NewPitch);
	FRotator NewRotation = FRotator(0.0f, CharacterState.CurrentHeadDelta.Y, CharacterState.CurrentHeadDelta.X);
	UpdateHeadRotation(NewRotation);
}

void ACharacter_Base::UpdateHeadRotation(FRotator HeadRotation)
{
	UAnimInstance* AnimInst = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (AnimInst && AnimInst->GetClass()->ImplementsInterface(UAnims::StaticClass()))
	{
		IAnims::Execute_OnUpdateHeadRotation(AnimInst, HeadRotation);
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
	AttachToActor(GetCurrentVehicle(), FAttachmentTransformRules::KeepRelativeTransform);
	Camera->bUsePawnControlRotation = false;
	Camera->SetRelativeRotation(FRotator());
	bUseControllerRotationYaw = false;

	APlayerController* PC = GetController<APlayerController>();
	static FProperty* ContextsProp = UEnhancedPlayerInput::StaticClass()->FindPropertyByName(FName("AppliedInputContexts"));
	void* ValuePtr = ContextsProp->ContainerPtrToValuePtr<void>(PC->PlayerInput);
	auto* MapPtr = static_cast<TMap<TObjectPtr<const UInputMappingContext>, int32>*>(ValuePtr);

	TArray<UInputMappingContext*> ContextsToModify;

	// 2. Iterate over the Map and fill the snapshot array
	for (auto& Pair : *MapPtr)
	{
		if (Pair.Value == 1)
		{
			ContextsToModify.Add(const_cast<UInputMappingContext*>(Pair.Key.Get()));
		}
	}

	// 3. Now iterate over the copy to safely perform your logic
	for (UInputMappingContext* IMC : ContextsToModify)
	{
		ManageIMC(IMC, nullptr, -1);
	}
}

void ACharacter_Base::CharacterExitVehicle()
{
	if (GetCurrentVehicle())
	{
		CharacterExitSeat(GetCurrentVehicle()->VehicleData->Seats[GetCSI()].DefaultCharacterContext);
		GetCurrentVehicle()->DropSeat(this, GetCSI());
		DetachFromActor(FDetachmentTransformRules::KeepRelativeTransform);

		FVector ExitLocation = CalculateSafeExitLocation(GetCurrentVehicle());
		SetActorLocation(ExitLocation);

		UpdateCharacterStance(ECharacterCurrentStance::Standing);
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Block);
		GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Vehicle, ECollisionResponse::ECR_Block);
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		Camera->bUsePawnControlRotation = true;
		bUseControllerRotationYaw = true;
		UpdateViewTarget(this, Camera);
		CharacterState.CharacterVehicleState = FCharacterVehicleState();
	}
}

void ACharacter_Base::CharacterEnterSeat(const FCharacterSeatContext& SeatContext)
{
	switch (GetCurrentVehicle()->GetVehicleData().Seats[GetCSI()].SeatRole)
	{
		case E_SeatRole::Driver:
		case E_SeatRole::Passenger:
			SetActorRelativeTransform(SeatContext.SeatTransform);
			break;
		case E_SeatRole::DriverGunner:
		case E_SeatRole::Gunner:
			UVehicleWeaponLogicComponent* VWLC = GetCurrentVehicle()->VehicleWeaponLogicComponent;
			const FVehicleWeaponInstanceData& VWID = VWLC->GetWeaponInstanceDataAtSlotInSeat(GetCSI(), VWLC->GetCWIForSeat(GetCSI()), VWLC->GetEquippedWeaponInSeat(GetCSI()).VehicleWeaponState.BaseWeaponRuntimeData.WeaponID);
			if (VWID.AttachmentInstanceData.bAttachCharacter)
			{
				TWeakObjectPtr<USkeletalMeshComponent> WeaponMesh = VWLC->VehicleWeaponSystem.Find(GetCSI())->VehicleWeaponSystemState.WeaponSystemMesh;
				FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, false);
				GetRootComponent()->AttachToComponent(WeaponMesh.Get(), AttachmentRules, FName("Test"));
				SetActorRelativeTransform(VWID.AttachmentInstanceData.CharacterTransform);
			}
			else
			{
				SetActorRelativeTransform(SeatContext.SeatTransform);
			}
			break;
	}

	UpdateCharacterStance(SeatContext.SeatStance);
	ManageIMC(nullptr, SeatContext.InputMappingContext, 1);
	UpdateCharacterMeshVisibility(SeatContext.bIsCharacterVisible);
	if (SeatContext.SeatHMD)
	{
		UpdateVehicleHUD(SeatContext.SeatHMD);
	}

	UpdateUI_EnterSeat();

}

void ACharacter_Base::CharacterExitSeat(const FCharacterSeatContext& SeatContext)
{
	if (GetAttachParentActor()->GetRootComponent() != GetRootComponent()->GetAttachParent())
	{
		GetRootComponent()->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
		AttachToActor(GetCurrentVehicle(), FAttachmentTransformRules::KeepRelativeTransform);
	}
	UpdateVehicleHUD(nullptr);
	ManageIMC(SeatContext.InputMappingContext, nullptr, 0);
}

FVector ACharacter_Base::CalculateSafeExitLocation(AActor* Vehicle)
{
	// Define exit points relative to the vehicle (Right, Left, Back)
	TArray<FVector> ExitOffsets;
	ExitOffsets.Add(FVector(0, 250, 50));   // Right
	ExitOffsets.Add(FVector(0, -250, 50));  // Left
	ExitOffsets.Add(FVector(-300, 0, 50));  // Back

	FVector BestLocation = Vehicle->GetActorLocation() + FVector(0, 0, 150); // Fallback: Above vehicle

	for (FVector Offset : ExitOffsets)
	{
		FVector TargetLocation = Vehicle->GetActorTransform().TransformPosition(Offset);

		//trace to see if the capsule fits there
		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(Vehicle);
		Params.AddIgnoredActor(this);
		bool bHit = GetWorld()->SweepSingleByChannel
		(
			Hit,
			TargetLocation + FVector(0, 0, 10),
			TargetLocation,
			FQuat::Identity,
			ECC_Visibility,
			GetCapsuleComponent()->GetCollisionShape(),
			Params
		);
		if (!bHit)
		{
			return TargetLocation;
		}
	}
	return BestLocation;
}

void ACharacter_Base::UpdateSeatIndexes(int32 NewLSI, int32 NewCSI, int32 NewNSI)
{
	CharacterState.CharacterVehicleState.LSI = NewLSI;
	CharacterState.CharacterVehicleState.CSI = NewCSI;
	CharacterState.CharacterVehicleState.NSI = NewNSI;
}

void ACharacter_Base::UpdateUI_EnterSeat()
{
	//sync vehicle states for hud
	switch (GetCurrentVehicle()->VehicleData->Seats[GetCSI()].SeatRole)
	{
		case E_SeatRole::Driver:
			GetLocalPlayerHUDSystem()->UpdateSpeedHUD_Vehicle(GetCurrentVehicle()->GetVelocity().Size());
			break;
		case E_SeatRole::Gunner:
			break;
		case E_SeatRole::DriverGunner:
			//HUD
			GetLocalPlayerHUDSystem()->UpdateSpeedHUD_Vehicle(GetCurrentVehicle()->GetVelocity().Size());

			//turrets/heading
			if (GetCurrentVehicle()->VehicleCurrentState.SeatStates[GetCSI()].ActiveCamera)
			{
				GetLocalPlayerHUDSystem()->UpdateCompassHUD_Vehicle(GetCurrentVehicle()->VehicleCurrentState.SeatStates[GetCSI()].ActiveCamera->GetComponentRotation().Yaw);
				GetLocalPlayerHUDSystem()->HandleTurretRotationUpdate(GetCurrentVehicle()->VehicleCurrentState.SeatStates[GetCSI()].ActiveCamera->GetComponentRotation().Yaw);
			}
			if (GetCurrentVehicle()->VehicleData->Seats[GetCSI()].AvailableItems.ControlledTurretIndexes.Num())
			{
				const int32& CTI = GetCurrentVehicle()->GetControlledTurret(GetCSI());

				GetLocalPlayerHUDSystem()->HandleTurretPitchUpdate
				(
					GetCurrentVehicle()->VehicleData->Turrets[CTI].TurretPitch.TurretMinMax.GetLowerBoundValue(),
					GetCurrentVehicle()->VehicleData->Turrets[CTI].TurretPitch.TurretMinMax.GetUpperBoundValue(),
					GetCurrentVehicle()->VehicleWeaponLogicComponent->TurretStates[CTI].CurrentTurretPitch
				);
			}

			break;
	}
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

void ACharacter_Base::BindInputAction(class UInputComponent* PlayerInputComponent, const UInputAction* IA, ETriggerEvent InputType, UObject* TargetObject, FName InputFunctionName)
{
	if (!TargetObject) TargetObject = this;
	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent_Player);
	if (UFunction* Function = TargetObject->FindFunction(InputFunctionName))
	{
		//FEnhancedInputActionHandlerDynamicSignature Delegate;
		//Delegate.BindUFunction(this, InputFunctionName);
		EnhancedInput->BindAction(IA, InputType, TargetObject, InputFunctionName);
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
	//free looking? (make sure its correctly managed this time)
	if (CharacterState.CharacterVehicleState.inVehicle)
	{
		const FSeatData& OccupiedSeatData = GetCurrentVehicle()->VehicleData->Seats[GetCSI()];
		if (OccupiedSeatData.ViewMethod == E_ViewMethod::Windowed)
		{
			TArray<AActor*> Actors;
			Actors.Add(GetCurrentVehicle());
			TWeakObjectPtr<UVehicleWeaponLogicComponent> VWLC = GetCurrentVehicle()->VehicleWeaponLogicComponent;
			const FBaseWeaponData& StaticWeaponData = VWLC->GetBaseWeaponDataInSlot(GetCSI(), VWLC->GetCWIForSeat(GetCSI()));
			FVehicleWeapon_Runtime& CurrentWeapon = VWLC->GetEquippedWeaponInSeat(GetCSI());
			FTransform TraceTransform;
			switch (CurrentWeapon.VehicleWeaponInstanceData.WindowedAimAnchor)
			{
				case EWindowedAimAnchor::Hull:
					FVector StartLocation = GetMesh()->GetSocketLocation(FName("FixedCamera"));
					FQuat FixedRotation = GetActorQuat();
					TraceTransform = FTransform(FixedRotation, StartLocation);
					break;
				case EWindowedAimAnchor::Turret:
				case EWindowedAimAnchor::FreeAim:
					TraceTransform = Camera->GetComponentTransform();
					break;
			}
			VWLC->UpdateSeatRangefinder(GetCSI(), TraceTransform, Actors);
			if (IsLocallyControlled())
			{
				//update HMD reticle
				FHitResult& HitResult = VWLC->VehicleWeaponSystem.Find(GetCSI())->VehicleWeaponSystemState.EquippedWeaponState.RaycastData.RangefinderData;
				UWidgetComponent* SeatHUDComp = GetCurrentVehicle()->VehicleCurrentState.SeatStates[GetCSI()].SeatHUDComponent;
				if (SeatHUDComp)
				{
					UStaticMeshComponent* Quad = VWLC->VehicleWeaponSystem.Find(GetCSI())->VehicleWeaponSystemState.ReticleQuad.Get();
					USceneComponent* HUDGlass = Quad->GetAttachParent();
					FVector EyePos = Camera->GetComponentLocation();
					FVector TargetPos = HitResult.bBlockingHit ? HitResult.ImpactPoint : TraceTransform.GetLocation() + (TraceTransform.GetUnitAxis(EAxis::X) * 100000.0f);
					FVector IntersectionPoint = FMath::LinePlaneIntersection(EyePos, TargetPos, SeatHUDComp->GetComponentLocation(), SeatHUDComp->GetForwardVector());
					Quad->SetWorldLocation(IntersectionPoint);
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
		if (!HUDSub->CurrentVehicleHMD && HUDClass)
		{
			HUDSub->SpawnVehicleSeatHUD(HUDClass);
		}
		else if (!HUDClass && HUDSub->CurrentVehicleHMD)
		{
			HUDSub->CurrentVehicleHMD->RemoveFromParent();
			HUDSub->CurrentVehicleHMD = nullptr;
		}
	}
}

TMap<TObjectPtr<const UInputMappingContext>, int32> ACharacter_Base::DebugCurrentIMC()
{
	APlayerController* PC = GetController<APlayerController>();
	if (!PC || !PC->PlayerInput) return TMap<TObjectPtr<const UInputMappingContext>, int32>();

	static FProperty* ContextsProp = UEnhancedPlayerInput::StaticClass()->FindPropertyByName(FName("AppliedInputContexts"));

	if (ContextsProp)
	{
		void* ValuePtr = ContextsProp->ContainerPtrToValuePtr<void>(PC->PlayerInput);

		auto* MapPtr = static_cast<TMap<TObjectPtr<const UInputMappingContext>, int32>*>(ValuePtr);

		if (MapPtr)
		{
			return *MapPtr;
		}
	}

	return TMap<TObjectPtr<const UInputMappingContext>, int32>();
}

TArray<FString> ACharacter_Base::DebugCurrentIMCNames()
{
	TArray<FString> IMCNames;

	APlayerController* PC = GetController<APlayerController>();
	if (!PC || !PC->PlayerInput) return IMCNames;

	// Cache the property lookup for efficiency
	static FProperty* ContextsProp = UEnhancedPlayerInput::StaticClass()->FindPropertyByName(FName("AppliedInputContexts"));

	if (ContextsProp)
	{
		void* ValuePtr = ContextsProp->ContainerPtrToValuePtr<void>(PC->PlayerInput);
		auto* MapPtr = static_cast<TMap<TObjectPtr<const UInputMappingContext>, int32>*>(ValuePtr);

		if (MapPtr)
		{
			for (auto& Pair : *MapPtr)
			{
				if (Pair.Key.Get())
				{
					// Add the name of the IMC to our array
					IMCNames.Add(Pair.Key.Get()->GetName());
				}
			}
		}
	}

	return IMCNames;
}
