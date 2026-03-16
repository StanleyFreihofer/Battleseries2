#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "InputAction.h"
#include "Animation/SkeletalMeshActor.h"
#include "Camera/CameraActor.h"
#include "Data/Weapons/VehicleWeapons/Data_VehicleWeapon.h"
#include "Core/Weapons/Projectiles/Projectile_Base.h"
#include "Core/Weapons/WeaponLogicComponent.h"
#include "Seat_Base.generated.h"

class ACharacter_Base;
class AVehicle_Base;

UCLASS()
class BATTLESERIES2_API ASeat_Base : public ACharacter		//<-- character in order to inherit hiearchy of regular/playable characters
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASeat_Base();

	//COMPONENTS
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	USpringArmComponent* SpringArm;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UCameraComponent* Camera;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UWeaponLogicComponent* WeaponLogicComponent;

	//VARIABLES

	//Runtime
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Runtime")
	bool isOccupied;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Runtime")
	int32 SeatIndex;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Runtime")
	AVehicle_Base* Vehicle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Runtime")
	ACharacter_Base* Character;


	//Input Actions
	//Input Action: Generic
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_ChangeSeat;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_ExitVehicle;
	//Input Action: Gunner
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_Shoot;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_SwitchWeapon;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_ControlTurret;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_ToggleOptic;
	//Input Action: Driver, Generic
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_Throttle;
	//Input Action: Driver, Ground Vehicle
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_Steer;

	//FUNCTIONS
	//INITIALIZATION FUNCTIONS

	//RUNTIME FUNCTIONS

	//Handle Input Functions


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
