#include "Utilities/ProjectilePoolSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Utilities/BS2FunctionLibrary.h"
#include "Core/Weapons/Projectiles/Projectile_Base.h"
#include "Data/Weapons/Data_Projectile.h"

void UProjectilePoolSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	//
	//THIS FUNCTION HAPPENS TOO EARLY TO SPAWN PROJECTILES HERE
}

void UProjectilePoolSubsystem::Tick(float DeltaTime)
{
	UpdateSimulatedProjectiles(DeltaTime);
}

TStatId UProjectilePoolSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UProjectilePoolSubsystem, STATGROUP_Tickables);
}

void UProjectilePoolSubsystem::SpawnPoolofProjectile(FName MunitionID, int32 PoolSize)
{
	FProjectilePoolEntry PoolEntry;

	const FProjectileData* ProjectileData = UBS2FunctionLibrary::GetDataSubsystem(this)->GetProjectileDataRow(MunitionID);

	for (int32 i = 0; i < PoolSize; i++)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ProjectilePoolSubsystem::Initialize] Projectile ID = %s"), *MunitionID.ToString());

		AProjectile_Base* NewProjectile = GetWorld()->SpawnActor<AProjectile_Base>(AProjectile_Base::StaticClass());
		UE_LOG(LogTemp, Warning, TEXT("[ProjectilePoolSubsystem::Initialize] Projectile ActorName: %s"), *NewProjectile->GetName());
		NewProjectile->SetProjectileAndInit(MunitionID, false);
		NewProjectile->SetActorHiddenInGame(true);
		NewProjectile->SetActorEnableCollision(false);
		//Projectile->GetRootComponent()->SetActive(false);
		PoolEntry.PooledProjectiles.Add(NewProjectile);
		//Projectile->RuntimeContext = nullptr; // Clear runtime context
	}
	ProjectileObjectPools.Add(MunitionID, PoolEntry);
}

TWeakObjectPtr<AProjectile_Base> UProjectilePoolSubsystem::AcquireProjectileFromPool(FName MunitionID)
{
	if (!ProjectileObjectPools.Contains(MunitionID))
	{
		UE_LOG(LogTemp, Error, TEXT("[ProjectilePoolSubsystem::AcquireProjectileFromPool] Invalid ProjectileID: %s"), *MunitionID.ToString());
		return nullptr;
	}

	FProjectilePoolEntry& Pool = ProjectileObjectPools[MunitionID];
	TWeakObjectPtr<AProjectile_Base> NewProjectile = nullptr;

	if (Pool.PooledProjectiles.Num() > 0)
	{
		NewProjectile = Pool.PooledProjectiles[0];
		Pool.PooledProjectiles.RemoveAt(0);
	}

 	NewProjectile->SetActorHiddenInGame(false);
	NewProjectile->SetActorEnableCollision(true);
	NewProjectile->SetActorTickEnabled(true);
	NewProjectile->ProjectileMovementComponent->SetComponentTickEnabled(true);
	return NewProjectile; 
}

void UProjectilePoolSubsystem::ReturnProjectileToPool(TWeakObjectPtr<AProjectile_Base> Projectile)
{
	//attach or Detach based on runtime context
	if (Projectile->ProjectileState.PreFlightContext.AttachedComponent || Projectile->GetParentComponent() != nullptr)
	{   
		UE_LOG(LogTemp, Warning, TEXT("[ProjectilePoolSubsystem::ReturnProjectileToPool] DetachFromActor"));
		Projectile->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	}

	Projectile->SetActorHiddenInGame(true);
	Projectile->SetActorEnableCollision(false);
	Projectile->SetActorTickEnabled(false);
	Projectile->ProjectileMovementComponent->SetComponentTickEnabled(false);

	FName& MunitionID = Projectile->ProjectileState.MunitionID;
	ProjectileObjectPools.FindOrAdd(MunitionID).PooledProjectiles.Add(Projectile);

	Projectile->SetActorLocation(FVector::ZeroVector);
}

void UProjectilePoolSubsystem::AddNewSimProjectile(const FSimProjectile_Runtime& NewProjectileSim)
{
	SimulatedProjectiles.Add(NewProjectileSim);
}

void UProjectilePoolSubsystem::UpdateSimulatedProjectiles(float DeltaSeconds)
{
	FVector NewLocation;
	FVector NewVelocity;
	FCollisionQueryParams Params;
	FHitResult OutHit;
	Params.bTraceComplex = true;

	for (int32 i = SimulatedProjectiles.Num() - 1; i >= 0; i--)
	{
		// 1. Get a COPY or be very careful with the reference
		FSimProjectile_Runtime& Sim = SimulatedProjectiles[i];

		NewVelocity = CalculateDrop(Sim.CurrentVelocity, Sim.GravityScale, DeltaSeconds);
		NewLocation = Sim.CurrentLocation + (NewVelocity * DeltaSeconds);

		bool bDidHit = GetWorld()->LineTraceSingleByChannel(OutHit, Sim.CurrentLocation, NewLocation, ECC_Visibility, Params);

		// 2. Only update the struct if we AREN'T about to delete it
		if (!bDidHit)
		{
			Sim.CurrentLocation = NewLocation;
			Sim.CurrentVelocity = NewVelocity;

			if (Sim.ProjectileMesh.IsValid())
			{
				Sim.ProjectileMesh->SetActorLocation(NewLocation);
			}

			// Debugging only for active projectiles
			DrawDebugLine(GetWorld(), Sim.CurrentLocation, NewLocation, FColor::Red, false, -1.f, 0, 1.f);
		}
		else
		{
			// 3. Handle the hit
			DrawDebugLine(GetWorld(), Sim.CurrentLocation, NewLocation, FColor::Yellow, false, -1.f, 0, 1.f);

			if (AActor* HitActor = OutHit.GetActor())
			{
				// Trigger damage here
			}

			// 4. Final log before the memory is freed
			UE_LOG(LogTemp, Warning, TEXT("Projectile Hit: %s"), *Sim.MunitionID.ToString());

			// 5. Remove and IMMEDIATELY move to next iteration
			SimulatedProjectiles.RemoveAt(i);
		}
	}
}

FVector UProjectilePoolSubsystem::CalculateDrop(FVector Velocity, float Gravity, float DeltaSeconds)
{
	FVector NewVelocity;
	NewVelocity = FVector(Velocity.X, Velocity.Y, Gravity * DeltaSeconds + Velocity.Z);
	return NewVelocity;
}
