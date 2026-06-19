#pragma once

#include "CoreMinimal.h"
#include "ProjectileTypes.generated.h"

class AStaticMeshActor;

USTRUCT(BlueprintType)
struct FProjectile_PreFlightContext
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)				// If null, this projectile is unmounted and goes to world 0 when released
	UPrimitiveComponent* AttachedComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)				// Socket or hardpoint name on the attached component
	FName AttachSocket = NAME_None;
};

USTRUCT(BlueprintType)
struct FSimProjectile_Runtime
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MunitionID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TWeakObjectPtr<class APlayerState> InstigatorPlayerState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TWeakObjectPtr<AStaticMeshActor> ProjectileMesh = nullptr;

	//movement state (updated everytick)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)     //the initial location of the projectile
	FVector FireOrigin = FVector();
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector CurrentLocation = FVector();
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector CurrentVelocity = FVector();
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GravityScale = 0.0f;

	//damage
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseDamage = 0.0f;						//copied from weapon?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)		//copied from weapon?
	UCurveFloat* DamageCurve = nullptr;		//multiply by base damage
};

USTRUCT(BlueprintType)
struct FActorProjectile_Runtime
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MunitionID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TWeakObjectPtr<class APlayerState> InstigatorPlayerState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)     //the origin location of the projectile (where fired from)
	FVector Origin = FVector();

	//movement state (updated everytick)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)     //the initial location of the projectile (usually while flight, the location to compare to)
	FVector InitialLocation = FVector();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FVector HomingTargetPoint = FVector();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GravityScale = 0.0f;

	//damage
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseDamage = 0.0f;						//copied from weapon?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)		//copied from weapon?
	UCurveFloat* DamageCurve = nullptr;		//multiply by base damage

	//Flight Plan State
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FlightStageIndex = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentStageTimer = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FProjectile_PreFlightContext PreFlightContext = FProjectile_PreFlightContext();
};