#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraComponent.h"
#include "Components/PointLightComponent.h"
#include "Data/Weapons/ProjectileTypes.h"
#include "Projectile_Base.generated.h"

struct FProjectileData;
struct FActorProjectile_Runtime;

UCLASS()
class BATTLESERIES2_API AProjectile_Base : public APawn
{
	GENERATED_BODY()

public:
	AProjectile_Base();
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UStaticMeshComponent* ProjectileMeshComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UPointLightComponent* PointLightComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UNiagaraComponent* NiagaraComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UProjectileMovementComponent* ProjectileMovementComponent;


	const FProjectileData* ProjectileData;

	//STATE
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FActorProjectile_Runtime ProjectileState;



	UFUNCTION(BlueprintCallable)
	void SetProjectileAndInit(FName InputProjectileID, bool ActivateImmediately);

	UFUNCTION(BlueprintCallable)
	void Init_ProjectileData();

	UFUNCTION(BlueprintCallable)
	void Init_Projectile();

	UFUNCTION(BlueprintCallable)
	void Init_ProjectileMesh(UStaticMesh* LoadedProjectileMesh);
	UFUNCTION(BlueprintCallable)
	void Init_RocketExhaustVFX();

	UFUNCTION(BlueprintCallable)
	void SetRuntimeContext(UPrimitiveComponent* AttachComponent, FName AttachSocket);
	UFUNCTION(BlueprintCallable)
	void EjectFromPylon();

	UFUNCTION(BlueprintCallable)
	void FireProjectile(FVector AimDirection);
	UFUNCTION(BlueprintCallable)
	void StartFlightPlan();
	UFUNCTION(BlueprintCallable)
	void UpdateFlightPlan(int32 FlightStageIndex);
	UFUNCTION(BlueprintCallable)
	void HandleFlightStageTransition(const FProjectileFlightStage& FlightStage);
	UFUNCTION(BlueprintCallable)
	void AdvanceFlightStage();
	UFUNCTION(BlueprintCallable)
	void HandleTransition_LimitedRange();
	UFUNCTION(BlueprintCallable)
	void HandleTransition_RangeToTarget();
	UFUNCTION(BlueprintCallable)
	void HandleTransition_Proximity2D();
	UFUNCTION(BlueprintCallable)
	FVector CalculateHomingToPoint(FVector TargetPoint);
	UFUNCTION(BlueprintCallable)
	void UpdateHomingPoint(FVector HomingPoint);
	UFUNCTION(BlueprintCallable)
	void UpdateManualHoming(FVector TargetPoint);
	UFUNCTION(BlueprintCallable)
	void UpdateManualHoming_GPS();
	UFUNCTION(BlueprintCallable)
	void StartGPSGuidance();
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	UFUNCTION(BlueprintCallable)
	void OnImpact();

protected:
	FTimerHandle StageTimerHandle;
	FTimerHandle ManualGuidanceTimerHandle;
	FTimerHandle CollisionTimerHandle;

};

