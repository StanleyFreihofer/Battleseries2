#include "Utilities/ProjectilePoolSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Utilities/DataManagerSubsystem.h"
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
	DataManager = GetWorld()->GetGameInstance()->GetSubsystem<UDataManagerSubsystem>();

	const FProjectileData* ProjectileData = DataManager->GetProjectileDataRow(MunitionID);
	switch (ProjectileData->ProjectileClassificationData.ProjectileType)
	{
		case EProjectileType::Rocket:
		case EProjectileType::Missile:
		case EProjectileType::Bomb:
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
			break;
	}
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
	bool bDidHit;
	FHitResult OutHit;

	for (int32 i = 0; i < SimulatedProjectiles.Num(); i++)
	{
		FSimProjectile_Runtime& SimulatedProjectile = SimulatedProjectiles[i];
		NewVelocity = CalculateDrop(SimulatedProjectile.CurrentVelocity, SimulatedProjectile.GravityScale, DeltaSeconds);
		NewLocation = SimulatedProjectile.CurrentLocation + NewVelocity * DeltaSeconds;

		bDidHit = GetWorld()->LineTraceSingleByChannel(OutHit, SimulatedProjectile.CurrentLocation, NewLocation, ECC_Visibility, Params);
		DrawDebugLine(GetWorld(), SimulatedProjectile.CurrentLocation, NewLocation, bDidHit ? FColor::Yellow : FColor::Red, false, -1.f, 0, 1.f);

		SimulatedProjectile.CurrentLocation = NewLocation;
		SimulatedProjectile.CurrentVelocity = NewVelocity;
		DrawDebugSphere(GetWorld(), NewLocation, 10.f, 8, FColor::Blue, false, -1.f);
		if (SimulatedProjectile.ProjectileMesh.Get())
		{
			SimulatedProjectile.ProjectileMesh->SetActorLocation(NewLocation);
		}

		if (bDidHit)
		{
			AActor* HitActor = OutHit.GetActor();
			if (HitActor)
			{
				//apply damage, etc
			}
			UE_LOG(LogTemp, Error, TEXT("[ProjectilePoolSubsystem::UpdateSimulatedProjectiles] ProjectileID = %s"), *SimulatedProjectile.MunitionID.ToString());
			/**
			const FProjectileData* ProjectileData = DataManager->GetProjectileDataRow(SimulatedProjectile.MunitionID);
			UNiagaraSystem* SelectedVFX = ProjectileData->ProjectileVisualData.ImpactVFX.LoadSynchronous();
			if (SelectedVFX)
			{
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), SelectedVFX, OutHit.ImpactPoint, OutHit.ImpactNormal.Rotation());
			}
			**/
			SimulatedProjectiles.RemoveAt(i);
			continue;
		}
	}
}

FVector UProjectilePoolSubsystem::CalculateDrop(FVector Velocity, float Gravity, float DeltaSeconds)
{
	FVector NewVelocity;
	NewVelocity = FVector(Velocity.X, Velocity.Y, Gravity * DeltaSeconds + Velocity.Z);
	return NewVelocity;
}
