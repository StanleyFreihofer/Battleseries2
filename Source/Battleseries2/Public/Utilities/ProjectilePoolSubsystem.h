#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ProjectilePoolSubsystem.generated.h"

class UDataManagerSubsystem;
class AProjectile_Base;
struct FSimProjectile_Runtime;

/**
* ROLES:
* 1. Updates and calculates "simulated"/non-object projectiles from creation to impact on tick
* 2. Handles pooling and lifetime of projectile objects
**/

USTRUCT(BlueprintType)
struct FProjectilePoolEntry
{
    GENERATED_BODY()

    // The actual pooled projectiles ACTORS
    UPROPERTY(EditAnywhere)
    TArray<TWeakObjectPtr<AProjectile_Base>> PooledProjectiles;
};

UCLASS(Blueprintable, BlueprintType)
class BATTLESERIES2_API UProjectilePoolSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:

    //map of projectile ID -> pool entry
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, FProjectilePoolEntry> ProjectileObjectPools;

    //default pool size per projectile type
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DefaultPoolSize = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FSimProjectile_Runtime> SimulatedProjectiles;

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    virtual void Tick(float DeltaTime);

    virtual TStatId GetStatId() const override;
  
    UFUNCTION(BlueprintCallable)
    void SpawnPoolofProjectile(FName ProjectileID, int32 PoolSize);

    //use this NOT TO FIRE OR MOUNT a projectile but to simply get/acquire it
    UFUNCTION()
    TWeakObjectPtr<AProjectile_Base> AcquireProjectileFromPool(FName ProjectileID);

    UFUNCTION()
    void ReturnProjectileToPool(TWeakObjectPtr<AProjectile_Base> Projectile);

    UFUNCTION(BlueprintCallable)
    void AddNewSimProjectile(const FSimProjectile_Runtime& NewProjectileSim);

    UFUNCTION(BlueprintCallable)
    void UpdateSimulatedProjectiles(float DeltaSeconds);

    UFUNCTION(BlueprintCallable)
    FVector CalculateDrop(FVector Velocity, float Gravity, float DeltaSeconds);

private:
    UDataManagerSubsystem* DataManager;

};