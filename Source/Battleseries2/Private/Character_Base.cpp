// Fill out your copyright notice in the Description page of Project Settings.

#include "Character_Base.h"
#include "Vehicle_Base.h"
#include "Data/Vehicles/VehicleTypes.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "Camera/CameraActor.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedPlayerInput.h"
#include "Data/Characters/CharacterDefaults.h"
#include "Data/Items/Weapons/Data_Weapon.h"
#include "Data/Items/Weapons/WeaponTypes.h"
#include "Core/Weapons/VehicleWeaponLogicComponent.h"
#include "Core/Weapons/LoadoutManager.h"
#include "Core/UI/VehicleHUDs/UW_HUD_Vehicle_Base.h"
#include "Core/UI/GameplayHUDs/UW_HUD_Status_Base.h"
#include "Core/PlayerController_Base.h"
#include "Core/Vehicles/CharacterVehicleManager.h"
#include "Core/Weapons/Projectiles/Projectile_Base.h"
#include "Utilities/I_Interact.h"
#include "Utilities/HUDSubsystem.h"
#include "Utilities/DataManagerSubsystem.h"
#include "Utilities/BS2FunctionLibrary.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
ACharacter_Base::ACharacter_Base(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	FPCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	FPArmsSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	FPLegsSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("LegsSpringArm"));
	FPArms = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FPArms"));
	FPLegs = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FPLegs"));
	LoadoutManager = CreateDefaultSubobject<ULoadoutManager>(TEXT("LoadoutManager"));
	VehicleManager = CreateDefaultSubobject<UCharacterVehicleManager>(TEXT("VehicleManager"));
	FPArmsSpringArm->SetupAttachment(GetCapsuleComponent());
	FPLegsSpringArm->SetupAttachment(GetCapsuleComponent());
	FPArms->SetupAttachment(FPArmsSpringArm);
	FPLegs->SetupAttachment(FPLegsSpringArm);
	FPCamera->SetupAttachment(FPArms, FName("Camera"));
	FPCamera->bUsePawnControlRotation = true;
	GetMesh()->bOwnerNoSee = true;
	FPArms->bOnlyOwnerSee = true;
	FPLegs->bOnlyOwnerSee = true;
}

// Called when the game starts or when spawned
void ACharacter_Base::BeginPlay()
{
	Super::BeginPlay();
	Init_Character();
}

void ACharacter_Base::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	if (APlayerController* PC = Cast<APlayerController>(NewController))
	{
		GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
		{
			Init_PlayerCharacter();
			LoadoutManager->Init_Loadout(CharacterStartingData.StartingLoadout);
		});
	}
}

// Called every frame
void ACharacter_Base::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	InteractTrace();
}

// Called to bind functionality to input
void ACharacter_Base::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ACharacter_Base::Init_Character()
{
	//called on spawn into game I imagine.
	//how to handle different spawn contexts (spawn in vehicle for example)

	Init_CharacterMovement();
	if (IsLocallyControlled())
	{
		//Init_PlayerCharacter();
	}


}

void ACharacter_Base::Init_PlayerCharacter()
{
	APlayerController* PC = Cast<APlayerController>(GetController());

	if (!PC) { return; }

	UBS2FunctionLibrary::GetDataSubsystem(this)->GetCharacterDefaults()->DefaultIMC.LoadSynchronous();
	UBS2FunctionLibrary::GetDataSubsystem(this)->GetCharacterDefaults()->DefaultGameplayIMC.LoadSynchronous();
	//IMC
	ManageIMC(nullptr, UBS2FunctionLibrary::GetDataSubsystem(this)->GetCharacterDefaults()->DefaultIMC.Get(), 0);
	ManageIMC(nullptr, UBS2FunctionLibrary::GetDataSubsystem(this)->GetCharacterDefaults()->DefaultGameplayIMC.Get(), 1);

	FInputModeGameOnly GameMode;
	PC->SetInputMode(GameMode);
	PC->bShowMouseCursor = false;

	//HUD
	UBS2FunctionLibrary::GetHUDSubsystem(this)->SpawnStatusHUD(UBS2FunctionLibrary::GetDataSubsystem(this)->GetCharacterDefaults()->StatusHUDClass.Get());
}

void ACharacter_Base::Init_CharacterMovement()
{
	FCharacterMovementData& CharacterMovementData = UBS2FunctionLibrary::GetDataSubsystem(this)->GetCharacterDefaults()->CharacterMovementData;
	//default character movement comp data
	GetCharacterMovement()->MaxAcceleration = CharacterMovementData.CharacterMovementDefaults.MaxAcceleration;
	GetCharacterMovement()->MaxWalkSpeed = CharacterMovementData.CharacterMovementDefaults.MaxWalkSpeed;
	GetCharacterMovement()->MaxWalkSpeedCrouched = CharacterMovementData.CharacterMovementDefaults.MaxWalkSpeedCrouched;

	CharacterState.CharacterMovementState.canSprint = CharacterMovementData.canSprint;
	CharacterState.CharacterMovementState.CurrentStamina = CharacterMovementData.StaminaDuration;
}

#pragma region Input

void ACharacter_Base::Input_Look(FVector2D InputAxisValue)
{
	if (!VehicleManager->GetInVehicle())
	{
		AddControllerYawInput(InputAxisValue.X);
		AddControllerPitchInput(InputAxisValue.Y);
	}
	else
	{
		switch (VehicleManager->GetCurrentVehicle()->VehicleData->Seats[VehicleManager->GetCSI()].DefaultCharacterContext.CharacterRotationMethod)
		{
			case EControlRotationMethod::Freelook:
				Freelook(InputAxisValue);
				break;
		}
	}

}

void ACharacter_Base::Input_Move(FVector2D InputAxisValue)
{
	if (VehicleManager->GetInVehicle()) { return; }

	AddMovementInput(FVector(GetActorForwardVector()), InputAxisValue.Y);
	AddMovementInput(FVector(GetActorRightVector()), InputAxisValue.X);


	if (CharacterState.CharacterMovementState.CurrentMovementMode == ECharacterMovementMode::Idle)
	{
		UpdateMovementMode(ECharacterMovementMode::Walking);
	}
}

void ACharacter_Base::Input_Sprint(bool Sprint)
{
	//add functionality and data for sprinting in different stances (also if can sprint in those stances)
	FCharacterMovementState& CharacterMovementState = CharacterState.CharacterMovementState;
	FCharacterMovementData& CharacterMovementData = UBS2FunctionLibrary::GetDataSubsystem(this)->GetCharacterDefaults()->CharacterMovementData;
	if (Sprint)
	{
		UpdateMovementMode(ECharacterMovementMode::Sprinting);
	}
	else if (CharacterMovementState.CurrentMovementMode == ECharacterMovementMode::Sprinting)
	{
		UpdateMovementMode(ECharacterMovementMode::Walking);
	}
}

void ACharacter_Base::Input_Interact()
{
	if (VehicleManager->GetInVehicle())	{ return; }
	StartInteract();
}

void ACharacter_Base::Input_ShootWeapon_Vehicle()
{
	TWeakObjectPtr<AProjectile_Base> FiredProjectile;
	switch (VehicleManager->GetCurrentVehicle()->VehicleData->Seats[VehicleManager->GetCSI()].SeatRole)
	{
		case E_SeatRole::DriverGunner:
		case E_SeatRole::Gunner:
			FiredProjectile = VehicleManager->GetCurrentVehicle()->VehicleWeaponLogicComponent->HandleStartFire(VehicleManager->GetCSI());
			break;
	}

	if (FiredProjectile.IsValid())
	{
		//check whatever data to see if we use projectile as view target and or control it (TV Missile)
		//set as controlled whatever, update state to route input to projectile rather than vehicle
	}
}

void ACharacter_Base::Input_SwitchWeapon_Vehicle()
{
	bool bSwitched = VehicleManager->GetCurrentVehicle()->VehicleWeaponLogicComponent->SwitchWeapon(VehicleManager->GetCSI());
	if (!bSwitched)
	{
		return;
	}
	const FSeatData& SeatData = VehicleManager->GetCurrentVehicle()->VehicleData->Seats[VehicleManager->GetCSI()];
	VehicleManager->GetCurrentVehicle()->HandleViewMethod(this, SeatData);
}

#pragma endregion

#pragma region Interaction

void ACharacter_Base::InteractTrace()
{
	if (!GetController() || VehicleManager->GetInVehicle())
	{
		return;
	}

	TArray<FHitResult> HitResults;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);
	const float& TraceDistance = UBS2FunctionLibrary::GetDataSubsystem(GetWorld())->GetCharacterDefaults()->TraceDistance;
	const float& InteractionDistance = UBS2FunctionLibrary::GetDataSubsystem(GetWorld())->GetCharacterDefaults()->InteractionDistance;
	UBS2FunctionLibrary::PerformSphereTraceMulti(GetWorld(), FPCamera->GetComponentTransform(), HitResults, ActorsToIgnore, 1.0f, TraceDistance, false);
	if (HitResults.Num() > 0 && HitResults[0].GetActor()->GetClass()->ImplementsInterface(UInteract::StaticClass()) && HitResults[0].Distance <= InteractionDistance)
	{
		CharacterState.InteractionState.HitInteractable = HitResults[0].GetActor();
		if (!CharacterState.InteractionState.HitInteractable.Get()) { return; }
		IInteract::Execute_HoverInteraction(CharacterState.InteractionState.HitInteractable.Get(), true);
	}
	else if (CharacterState.InteractionState.HitInteractable.IsValid())
	{
		IInteract::Execute_HoverInteraction(CharacterState.InteractionState.HitInteractable.Get(), false);
		CharacterState.InteractionState.HitInteractable = nullptr;
	}
}

void ACharacter_Base::StartInteract()
{
	UObject* TargetObject = CharacterState.InteractionState.HitInteractable.Get();
	if (!TargetObject)
	{
		return;
	}
	if (!TargetObject->GetClass()->ImplementsInterface(UInteract::StaticClass()))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s does not implement UInteract interface!"), *TargetObject->GetName());
		return;
	}
	EInteractType CurrentType = IInteract::Execute_GetInteractionType(TargetObject);
	UE_LOG(LogTemp, Warning, TEXT("Interaction Type Returned: %d"), (int32)CurrentType);
	switch (CurrentType)
	{
		case EInteractType::Press:
			AttemptInteract();
			break;
		case EInteractType::Hold:
			if (!GetWorld()->GetTimerManager().IsTimerActive(CharacterState.InteractionState.InteractTimer))
			{
				GetWorld()->GetTimerManager().SetTimer(CharacterState.InteractionState.InteractTimer, this, &ACharacter_Base::AttemptInteract, 1.0f, false);
			}
			break;
		case EInteractType::Passive:
			break;
	}
}

void ACharacter_Base::AttemptInteract()
{
	IInteract::Execute_Interact(CharacterState.InteractionState.HitInteractable.Get(), this);
}

void ACharacter_Base::StopInteract()
{
	GetWorld()->GetTimerManager().ClearTimer(CharacterState.InteractionState.InteractTimer);
}

#pragma endregion

#pragma region Movement

#pragma region Sprint/Stamina

void ACharacter_Base::DepleteStamina()
{
	FCharacterMovementState& CharacterMovementState = CharacterState.CharacterMovementState;
	CharacterMovementState.CurrentStamina = CharacterMovementState.CurrentStamina - GetWorld()->GetDeltaSeconds();
	if (CharacterMovementState.CurrentStamina <= 0.0f)
	{
		CharacterMovementState.canSprint = false;
		UpdateMovementMode(ECharacterMovementMode::Walking);
	}
}

void ACharacter_Base::ReplenishStamina()
{
	FCharacterMovementData& CharacterMovementData = UBS2FunctionLibrary::GetDataSubsystem(this)->GetCharacterDefaults()->CharacterMovementData;
	FCharacterMovementState& CharacterMovementState = CharacterState.CharacterMovementState;

	CharacterMovementState.CurrentStamina = CharacterMovementState.CurrentStamina + GetWorld()->GetDeltaSeconds();
	if (CharacterMovementState.CurrentStamina > 0.0f)
	{
		CharacterMovementState.canSprint = true;
		if (CharacterMovementState.CurrentStamina >= CharacterMovementData.StaminaDuration)
		{
			GetWorldTimerManager().ClearTimer(CharacterMovementState.SprintTimer);
		}
	}
}

void ACharacter_Base::StartSprint()
{
	FCharacterMovementState& CharacterMovementState = CharacterState.CharacterMovementState;
	FCharacterMovementData& CharacterMovementData = UBS2FunctionLibrary::GetDataSubsystem(this)->GetCharacterDefaults()->CharacterMovementData;

	if (CharacterMovementState.CurrentStamina <= 0.0f) { return; }		//CharacterMovementState.canSprint

	GetCharacterMovement()->MaxWalkSpeed = CharacterMovementData.MaxSprintSpeed;

	//start decrementing stamina value
	if (CharacterMovementState.CurrentStamina > 0.0f && CharacterMovementState.CurrentMovementMode != ECharacterMovementMode::Sprinting)
	{
		GetWorldTimerManager().SetTimer(CharacterMovementState.SprintTimer, this, &ACharacter_Base::DepleteStamina, GetWorld()->GetDeltaSeconds(), true);
		CharacterMovementState.CurrentMovementMode = ECharacterMovementMode::Sprinting;
	}
}

void ACharacter_Base::StopSprint()
{
	FCharacterMovementData& CharacterMovementData = UBS2FunctionLibrary::GetDataSubsystem(this)->GetCharacterDefaults()->CharacterMovementData;

	GetCharacterMovement()->MaxWalkSpeed = CharacterMovementData.CharacterMovementDefaults.MaxWalkSpeed;
	
	//reset timer, start incrementing stamina value
	GetWorldTimerManager().SetTimer(CharacterState.CharacterMovementState.SprintTimer, this, &ACharacter_Base::ReplenishStamina, GetWorld()->GetDeltaSeconds(), true);
	//UpdateMovementMode(ECharacterMovementMode::Walking);
}

#pragma endregion

void ACharacter_Base::UpdateMovementMode(ECharacterMovementMode NewMode)
{
	ECharacterMovementMode& CurrentMovementMode = GetCurrentMovementMode();

	if (CurrentMovementMode == NewMode) { return; }

	switch (NewMode)
	{
		case ECharacterMovementMode::Idle:
		{
			if (CurrentMovementMode == ECharacterMovementMode::Sprinting)
			{
				StopSprint();
			}
			CurrentMovementMode = ECharacterMovementMode::Idle;
			break;
		}
		case ECharacterMovementMode::Walking:
		{
			if (CurrentMovementMode == ECharacterMovementMode::Sprinting)
			{
				StopSprint();
			}
			CurrentMovementMode = ECharacterMovementMode::Walking;
			break;
		}
		case ECharacterMovementMode::Sprinting:
		{
			if (LoadoutManager->CombatState.isAiming) { return; }
			else if (CurrentMovementMode == ECharacterMovementMode::Walking)
			{
				StartSprint();
			}
			break;
		}
	}
}

#pragma region StanceManagement

void ACharacter_Base::HandleUpdateStance(ECharacterStance NewStance)
{
	//undo previous stuff
	const FCharacterMovementData& CharacterMovementData = UBS2FunctionLibrary::GetDataSubsystem(this)->GetCharacterDefaults()->CharacterMovementData;
	ECharacterStance& CurrentStance = GetCurrentStance();
	if (NewStance == CurrentStance) { return; }
	if (!CharacterMovementData.canCrouch && NewStance == ECharacterStance::Crouching) { return; }
	if (!CharacterMovementData.canProne && NewStance == ECharacterStance::Proning) { return; }

	switch (CurrentStance)
	{
		case ECharacterStance::Standing:
			switch (NewStance)
			{
				case ECharacterStance::Crouching:
					Crouch();
					break;
				case ECharacterStance::Proning:
					Crouch();
					EnterProne();
					break;
			}
			break;
		case ECharacterStance::Crouching:
			switch (NewStance)
			{
				case ECharacterStance::Standing:
					UnCrouch();
					break;
				case ECharacterStance::Proning:
					EnterProne();
					break;
			}		
			break;
		case ECharacterStance::Proning:
			switch (NewStance)
			{
				case ECharacterStance::Crouching:
					ExitProne();
					Crouch();
					break;
			}
			break;
	}
	CharacterState.CharacterStanceState.CurrentStance = NewStance;
}

void ACharacter_Base::EnterProne()
{
	GetWorld()->GetTimerManager().SetTimer(CharacterState.CharacterStanceState.StanceTransitionTimer, this, &ACharacter_Base::InterpEnterProne_FP, GetWorld()->GetDeltaSeconds(), true);
}

void ACharacter_Base::ExitProne()
{
	GetWorld()->GetTimerManager().SetTimer(CharacterState.CharacterStanceState.StanceTransitionTimer, this, &ACharacter_Base::InterpExitProne_FP, GetWorld()->GetDeltaSeconds(), true);
}

void ACharacter_Base::InterpEnterProne_FP()
{
	float DeltaTime = GetWorld()->GetTimerManager().GetTimerElapsed(CharacterState.CharacterStanceState.StanceTransitionTimer);
	float InterpSpeed = 12.0f;
	FVector CurrentOffset = FPArmsSpringArm->GetRelativeLocation();
	const FVector TargetOffset = UBS2FunctionLibrary::GetDataSubsystem(this)->GetCharacterDefaults()->ProneFPHeight;
	FVector Offset = FMath::VInterpTo(CurrentOffset, TargetOffset, DeltaTime, InterpSpeed);
	FPArmsSpringArm->SetRelativeLocation(Offset);
	if (FVector::PointsAreNear(Offset, TargetOffset, 0.05f))
	{
		FPArmsSpringArm->SetRelativeLocation(TargetOffset);
		GetWorld()->GetTimerManager().ClearTimer(CharacterState.CharacterStanceState.StanceTransitionTimer);
	}
}

void ACharacter_Base::InterpExitProne_FP()
{
	float DeltaTime = GetWorld()->GetTimerManager().GetTimerElapsed(CharacterState.CharacterStanceState.StanceTransitionTimer);
	float InterpSpeed = 12.0f;
	FVector CurrentOffset = FPArmsSpringArm->GetRelativeLocation();
	FVector Offset = FMath::VInterpTo(CurrentOffset, FVector::ZeroVector, DeltaTime, InterpSpeed);
	FPArmsSpringArm->SetRelativeLocation(Offset);
	if (FVector::PointsAreNear(Offset, FVector::ZeroVector, 0.05f))
	{
		FPArmsSpringArm->SetRelativeLocation(FVector::ZeroVector);
		GetWorld()->GetTimerManager().ClearTimer(CharacterState.CharacterStanceState.StanceTransitionTimer);
	}
}

#pragma endregion

#pragma endregion

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

#pragma region Vehicle



#pragma endregion

void ACharacter_Base::UpdateViewTarget(TWeakObjectPtr<AActor> NewViewTarget, TWeakObjectPtr<UCameraComponent> CameraComponent)
{
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		CameraComponent.Get()->SetActive(true);
		PC->SetViewTarget(NewViewTarget.Get());
		UE_LOG(LogTemp, Warning, TEXT("[UpdateViewTarget] Target=%s Cam=%s CurrentPCViewTarget=%s"), *NewViewTarget->GetName(), *CameraComponent->GetName(), PC->GetViewTarget() ? *PC->GetViewTarget()->GetName() : TEXT("NULL"));
		//UpdateViewTarget_BP(NewViewTarget.Get(), CameraComponent.Get());
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
	if (VehicleManager->GetInVehicle())
	{
		OnFireReleased_Vehicle.Broadcast(VehicleManager->GetCSI());
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

# pragma region MeshVisibility

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

#pragma endregion



ECharacterStance& ACharacter_Base::GetCurrentStance()
{
	return CharacterState.CharacterStanceState.CurrentStance;
}

ECharacterMovementMode& ACharacter_Base::GetCurrentMovementMode()
{
	return CharacterState.CharacterMovementState.CurrentMovementMode;
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
