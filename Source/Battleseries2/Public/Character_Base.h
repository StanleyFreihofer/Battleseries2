// Fill out your copyright notice in the Description page of Project Settings.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Data/Characters/CharacterTypes.h"
#include "Data/Core/CoreTypes.h"
#include "Utilities/I_Anims.h"
#include "Character_Base.generated.h"

class USaveSubsystem;
class AVehicle_Base;
class UDataManagerSubsystem;
class UWeaponLogicComponent;
struct FCharacterSeatContext;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponInputReleased_Vehicle, int32, SeatIndex);

USTRUCT(BlueprintType)
struct FCharacterStartingData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instance")
	FName CharacterID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instance")
	FPlayerLoadoutConfig_Class StartingLoadout = FPlayerLoadoutConfig_Class();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instance")
	float StartingHealth = 100.0f;
};

UCLASS()
class BATTLESERIES2_API ACharacter_Base : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACharacter_Base(const FObjectInitializer& ObjectInitializer);

	//COMPONENTS
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UCameraComponent* FPCamera = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	USpringArmComponent* FPArmsSpringArm = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	USpringArmComponent* FPLegsSpringArm = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	USkeletalMeshComponent* FPArms = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	USkeletalMeshComponent* FPLegs = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UWeaponLogicComponent* WeaponManager;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnWeaponInputReleased_Vehicle OnFireReleased_Vehicle;

	//VARIABLES
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Runtime")
	FCharacterState CharacterState = FCharacterState();

	//Inputs
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputComponent* InputComponent_Player = nullptr;

	//FUNCTIONS
	UFUNCTION(BlueprintCallable)
	void Init_Character();
	UFUNCTION(BlueprintCallable)
	void Init_PlayerCharacter();

#pragma region Input
	UFUNCTION(BlueprintCallable)
	void Input_Look(FVector2D InputAxisValue);
	UFUNCTION(BlueprintCallable)
	void Input_Move(FVector2D InputAxisValue);
	UFUNCTION(BlueprintCallable)
	void Input_Interact();
	UFUNCTION(BlueprintCallable)
	void Input_ShootWeapon_Vehicle();
	UFUNCTION(BlueprintCallable)
	void Input_SwitchWeapon_Vehicle();
#pragma endregion

#pragma region Interaction
	UFUNCTION(BlueprintCallable)
	void InteractTrace();
	UFUNCTION(BlueprintCallable)
	void StartInteract();
	UFUNCTION(BlueprintCallable)
	void AttemptInteract();
	UFUNCTION(BlueprintCallable)
	void StopInteract();
#pragma endregion

	UFUNCTION(BlueprintCallable)
	void HandleUpdateStance(ECharacterStance NewStance);
	UFUNCTION(BlueprintCallable)
	void Freelook(FVector2D InputAxisValue);
	UFUNCTION(BlueprintCallable)
	void UpdateHeadRotation(FRotator HeadRotation);
	UFUNCTION(BlueprintCallable)
	void ManageinVehicleStatus(AVehicle_Base* Vehicle, bool In_Vehicle);
	UFUNCTION(BlueprintCallable)
	void UpdateSeatList(TArray<ACharacter_Base*> Characters);
	UFUNCTION(BlueprintCallable)
	void CharacterEnterVehicle();
	UFUNCTION(BlueprintCallable)
	void CharacterExitVehicle();
	UFUNCTION(BlueprintCallable)
	void CharacterEnterSeat(const FCharacterSeatContext& SeatContext);
	UFUNCTION(BlueprintCallable)
	void CharacterExitSeat(const FCharacterSeatContext& SeatContext);
	UFUNCTION(BlueprintCallable)
	FVector CalculateSafeExitLocation(AActor* Vehicle);
	UFUNCTION(BlueprintCallable)
	void UpdateSeatIndexes(int32 NewLSI, int32 NewCSI, int32 NewNSI);
	UFUNCTION(BlueprintCallable)
	void UpdateUI_EnterSeat();

	UFUNCTION()
	void UpdateViewTarget(TWeakObjectPtr<AActor> NewViewTarget, TWeakObjectPtr<UCameraComponent> CameraComponent);

	UFUNCTION(BlueprintCallable)
	void BindInputAction(class UInputComponent* PlayerInputComponent, const UInputAction* IA, ETriggerEvent InputType, UObject* TargetObject, FName InputFunctionName);

	UFUNCTION(BlueprintCallable)
	void HandleFireCompleted();

	UFUNCTION(BlueprintCallable)
	void ManageIMC(UInputMappingContext* IMC_ToRemove, UInputMappingContext* IMC_ToAdd, int32 IMCAddPriority);
	UFUNCTION(BlueprintCallable)
	void HideCharacterMesh();
	UFUNCTION(BlueprintCallable)
	void ShowCharacterMesh();
	UFUNCTION(BlueprintCallable)
	void UpdateCharacterMeshVisibility(bool ShowMesh);
	UFUNCTION(BlueprintCallable)
	void UpdateRangefinder_WindowedVehicle();

	UFUNCTION(BlueprintCallable)
	void UpdateVehicleHUD(TSubclassOf<UUserWidget> HUDClass);


	UFUNCTION(BlueprintCallable, BlueprintPure)
	AVehicle_Base* GetCurrentVehicle();
	UFUNCTION(BlueprintCallable, BlueprintPure)
	int32& GetCSI();

	TMap<TObjectPtr<const UInputMappingContext>, int32> DebugCurrentIMC();

	UFUNCTION(BlueprintCallable, Category = "Input|Debug")
	TArray<FString> DebugCurrentIMCNames();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	//Called upon possession by a PlayerController
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
