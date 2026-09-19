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
struct FBaseProjectileState
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)			//use to look up damage and what not
	FName MunitionID = NAME_None;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TWeakObjectPtr<class APlayerState> InstigatorPlayerState = nullptr;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)     //the initial location of the projectile (muzzle location)
	FVector FireOrigin = FVector();
};

USTRUCT(BlueprintType)
struct FSimProjectile_Runtime
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FBaseProjectileState BaseProjectileState = FBaseProjectileState();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TWeakObjectPtr<AStaticMeshActor> ProjectileMesh = nullptr;

	//movement state (updated everytick)
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FVector CurrentLocation = FVector();
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FVector CurrentVelocity = FVector();
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float GravityScale = 0.0f;
};

USTRUCT(BlueprintType)
struct FActorProjectile_Runtime
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FBaseProjectileState BaseProjectileState = FBaseProjectileState();

	//movement state (updated everytick)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)     //the initial location of the projectile (usually while flight, the location to compare to)
	FVector InitialLocation = FVector();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FVector HomingTargetPoint = FVector();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GravityScale = 0.0f;

	//Flight Plan State
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FlightStageIndex = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentStageTimer = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FProjectile_PreFlightContext PreFlightContext = FProjectile_PreFlightContext();
};