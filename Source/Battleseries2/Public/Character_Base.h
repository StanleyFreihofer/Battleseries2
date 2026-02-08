// Fill out your copyright notice in the Description page of Project Settings.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Data/Runtime/CharacterTypes.h"
#include "Character_Base.generated.h"

class AVehicle_Base;
struct FCharacterSeatContext;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponInputReleased_Vehicle, int32, SeatIndex);

UCLASS()
class BATTLESERIES2_API ACharacter_Base : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACharacter_Base();

	//COMPONENTS
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UCameraComponent* Camera = nullptr;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnWeaponInputReleased_Vehicle OnFireReleased_Vehicle;

	//VARIABLES
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Runtime")
	FCharacterState CharacterState = FCharacterState();

	//Inputs
	// Soft reference to the mapping context set in the Content Browser
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputComponent* InputComponent_Player = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TSoftObjectPtr<UInputMappingContext> DefaultIMCSoft = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_Fire;
	UInputMappingContext* DefaultIMC = nullptr;

	//FUNCTIONS
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
	void UpdateCharacterStance(ECharacterCurrentStance NewStance);

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
	UHUDSubsystem* GetLocalPlayerHUDSystem();
	UFUNCTION(BlueprintCallable, BlueprintPure)
	AVehicle_Base* GetCurrentVehicle();
	UFUNCTION(BlueprintCallable, BlueprintPure)
	int32& GetCSI();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	//Called upon possession by a PlayerController
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
