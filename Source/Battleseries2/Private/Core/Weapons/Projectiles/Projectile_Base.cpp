#include "Core/Weapons/Projectiles/Projectile_Base.h"
#include "Data/Weapons/Data_Projectile.h"
#include "Utilities/DataManagerSubsystem.h"
#include "Utilities/GameInstance_Base.h"
#include "Utilities/ProjectilePoolSubsystem.h"
#include "Utilities/BS2FunctionLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"

AProjectile_Base::AProjectile_Base()
{
	ProjectileMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMeshComponent"));
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	PointLightComponent = CreateDefaultSubobject<UPointLightComponent>(TEXT("PointLight"));
	NiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("RocketExhaustComponent"));
	NiagaraComponent->SetupAttachment(ProjectileMeshComponent);
	ProjectileMovementComponent->SetAutoActivate(false);				//<---should this really be in the constructor????
	NiagaraComponent->SetAutoActivate(false);
	RootComponent = ProjectileMeshComponent;
	ProjectileMeshComponent->SetNotifyRigidBodyCollision(true);
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->bInitialVelocityInLocalSpace = true;

}

void AProjectile_Base::BeginPlay()
{
	Super::BeginPlay();
}

void AProjectile_Base::SetProjectileAndInit(FName InputProjectileID, bool ActivateImmediately)
{
	ProjectileState.MunitionID = InputProjectileID;
	Init_ProjectileData();
}

void AProjectile_Base::Init_ProjectileData()
{
	const FProjectileData* ProjectileRow = UBS2FunctionLibrary::GetDataSubsystem(this)->GetProjectileDataRow(ProjectileState.MunitionID);
	ProjectileData = ProjectileRow;

	TArray<FSoftObjectPath> AssetsToLoad;
	AssetsToLoad.Add(ProjectileData->ProjectileVisualData.ProjectileMesh.ToSoftObjectPath());
	AssetsToLoad.Add(ProjectileData->ProjectileVisualData.RocketExhaust.ToSoftObjectPath());

	FStreamableManager* StreamableManager = Cast<UGameInstance_Base>(UGameplayStatics::GetGameInstance(this))->GetStreamableManager();
	StreamableManager->RequestAsyncLoad(AssetsToLoad, FStreamableDelegate::CreateUObject(this, &AProjectile_Base::Init_Projectile));
}

void AProjectile_Base::Init_Projectile()
{
	Init_ProjectileMesh(ProjectileData->ProjectileVisualData.ProjectileMesh.Get());
	Init_RocketExhaustVFX();
	//Init_ProjectileFlightData();
}

void AProjectile_Base::Init_ProjectileMesh(UStaticMesh* LoadedProjectileMesh)
{
	ProjectileMeshComponent->SetStaticMesh(LoadedProjectileMesh);
	ProjectileMeshComponent->SetSimulatePhysics(false);				//how are we handling physics sims for projectiles in general? (prototype in bp i guess)

}

void AProjectile_Base::Init_RocketExhaustVFX()
{
	UNiagaraSystem* ExhaustAsset = ProjectileData->ProjectileVisualData.RocketExhaust.Get();
	if (ExhaustAsset != nullptr)
	{
		NiagaraComponent->SetAsset(ExhaustAsset);
		NiagaraComponent->SetRelativeLocation(ProjectileData->ProjectileVisualData.ExhaustLocationOffset);
	}
}

void AProjectile_Base::SetRuntimeContext(UPrimitiveComponent* AttachComponent, FName AttachSocket)
{
	//PREFLIGHT CONTEXT
	if (AttachComponent)
	{
		this->AttachToComponent(AttachComponent, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), AttachSocket);
		ProjectileState.PreFlightContext.AttachedComponent = AttachComponent;
		ProjectileState.PreFlightContext.AttachSocket = AttachSocket;
		MoveIgnoreActorAdd(GetAttachParentActor());			//do this somewhere else????????????????????????????????????????????????????????????????????????????????
		ProjectileMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SetActorEnableCollision(false);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Projectile_Base::SetRuntimeContext] AttachComponent invalid, should detach happen here?"));
		ProjectileState.PreFlightContext.AttachedComponent = nullptr;
		ProjectileState.PreFlightContext.AttachSocket = NAME_None;
	}
}

void AProjectile_Base::EjectFromPylon()
{
	FVector VehicleVelocity = GetVelocity();					//FVector VehicleVelocity = GetAttachParentActor()->GetVelocity();
	FVector DownVector = -GetAttachParentActor()->GetRootComponent()->GetUpVector();
	FVector EjectionImpulse = DownVector * 850000.0f; // Scale to weapon mass
	FVector FrontPistonLoc = ProjectileMeshComponent->GetCenterOfMass() + (GetAttachParentActor()->GetRootComponent()->GetForwardVector() * 25.0f);

	UE_LOG(LogTemp, Warning, TEXT("[ProjectilePoolSubsystem::ReturnProjectileToPool] DetachFromActor"));
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	ProjectileMeshComponent->SetSimulatePhysics(true);
	ProjectileMeshComponent->SetPhysicsLinearVelocity(VehicleVelocity);
	ProjectileMeshComponent->AddImpulseAtLocation(EjectionImpulse, FrontPistonLoc);
}

void AProjectile_Base::FireProjectile(FVector AimDirection)
{
	if (ProjectileState.PreFlightContext.AttachedComponent || GetParentComponent() != nullptr)
	{
		EjectFromPylon();
	}

	ProjectileState.Origin = AimDirection;

	GetWorldTimerManager().SetTimer(CollisionTimerHandle, this, &AProjectile_Base::StartFlightPlan, 0.5f, true);
}

void AProjectile_Base::StartFlightPlan()
{
	ProjectileMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SetActorEnableCollision(true);
	if (ProjectileMeshComponent->IsSimulatingPhysics())
	{
		ProjectileMeshComponent->SetSimulatePhysics(false);
	}

	ProjectileMeshComponent->OnComponentHit.AddDynamic(this, &AProjectile_Base::OnHit);		//should not explode if not in use yet (hanging on rack for example)
	ProjectileMovementComponent->Velocity = ProjectileState.Origin * ProjectileData->ProjectileFlightPlan[0].GuidanceParams.InitialSpeed;
	ProjectileMovementComponent->Activate();
	NiagaraComponent->Activate();

	UpdateFlightPlan(0);
}

void AProjectile_Base::UpdateFlightPlan(int32 FlightStageIndex)
{
	const FProjectileFlightStage& FlightStage = ProjectileData->ProjectileFlightPlan[FlightStageIndex];

	//undo stuff from previous stage
	if (FlightStageIndex > 0)
	{
		const FProjectileFlightStage& PreviousFlightStage = ProjectileData->ProjectileFlightPlan[FlightStageIndex - 1];
		switch (PreviousFlightStage.BehaviorType)
		{
			case EProjectileGuidanceMethod::GuideToTarget:
				ProjectileMovementComponent->bIsHomingProjectile = false;
				break;
			case EProjectileGuidanceMethod::AutoGuideToPoint:
				GetWorld()->GetTimerManager().ClearTimer(ManualGuidanceTimerHandle);
				break;
			case EProjectileGuidanceMethod::Drop:
				ProjectileMeshComponent->SetSimulatePhysics(false);
				break;
		}
	}

	//guidance params
	ProjectileMovementComponent->InitialSpeed = FlightStage.GuidanceParams.InitialSpeed;
	ProjectileMovementComponent->MaxSpeed = FlightStage.GuidanceParams.MaxSpeed;
	ProjectileMovementComponent->ProjectileGravityScale = FlightStage.GuidanceParams.GravityScale;

	//behavior
	switch (FlightStage.BehaviorType)
	{
		case EProjectileGuidanceMethod::BallisticTrajectory:
			ProjectileMovementComponent->Velocity = ProjectileState.Origin * FlightStage.GuidanceParams.InitialSpeed;
			break;
		case EProjectileGuidanceMethod::PitchToAltitude:
			FVector UpwardPitch = FVector::UpVector * FlightStage.GuidanceParams.PitchForce;
			ProjectileMovementComponent->Velocity += UpwardPitch;
			break;
		case EProjectileGuidanceMethod::ManualGuideToPoint:	
			ProjectileMovementComponent->bIsHomingProjectile = false;		//does not guide toward an actor/component (uses custom function to guide towards a point)
			ProjectileMovementComponent->HomingAccelerationMagnitude = FlightStage.GuidanceParams.Acceleration;
			break;
		case EProjectileGuidanceMethod::AutoGuideToPoint:
			ProjectileMovementComponent->bIsHomingProjectile = false;
			ProjectileMovementComponent->HomingAccelerationMagnitude = FlightStage.GuidanceParams.Acceleration;
			StartGPSGuidance();
			break;
		case EProjectileGuidanceMethod::GuideToTarget:
			ProjectileMovementComponent->bIsHomingProjectile = true;
			ProjectileMovementComponent->HomingAccelerationMagnitude = FlightStage.GuidanceParams.Acceleration;
			break;
		case EProjectileGuidanceMethod::Drop:
			ProjectileMeshComponent->SetSimulatePhysics(true);
			break;
	}
	HandleFlightStageTransition(FlightStage);
}

#pragma region HandleStageTransition

void AProjectile_Base::HandleFlightStageTransition(const FProjectileFlightStage& FlightStage)
{
	switch (FlightStage.PrimaryTransitionCondition)
	{
		case ETransitionCondition::LimitedTime:
			GetWorld()->GetTimerManager().SetTimer(StageTimerHandle, this, &AProjectile_Base::AdvanceFlightStage, FlightStage.RequiredValue, false);
			break;
		case ETransitionCondition::LimitedRange:
			ProjectileState.InitialLocation = GetActorLocation();
			GetWorld()->GetTimerManager().SetTimer(StageTimerHandle, this, &AProjectile_Base::HandleTransition_LimitedRange, GetWorld()->GetDeltaSeconds(), true);
			break;
		case ETransitionCondition::Proximity2D:
			GetWorld()->GetTimerManager().SetTimer(StageTimerHandle, this, &AProjectile_Base::HandleTransition_Proximity2D, GetWorld()->GetDeltaSeconds(), true);
			break;
		case ETransitionCondition::RangeToTarget:
			GetWorld()->GetTimerManager().SetTimer(StageTimerHandle, this, &AProjectile_Base::HandleTransition_RangeToTarget, GetWorld()->GetDeltaSeconds(), true);
			break;
	}
}

void AProjectile_Base::AdvanceFlightStage()
{
	int32& CurrentFlightStage = ProjectileState.FlightStageIndex;
	CurrentFlightStage = (CurrentFlightStage + 1) % ProjectileData->ProjectileFlightPlan.Num();
	UpdateFlightPlan(CurrentFlightStage);
}

void AProjectile_Base::HandleTransition_LimitedRange()
{
	FVector EndPoint = GetActorLocation();
	float Distance = FVector::Dist(ProjectileState.InitialLocation, EndPoint);
	if (Distance >= ProjectileData->ProjectileFlightPlan[ProjectileState.FlightStageIndex].RequiredValue)
	{
		GetWorld()->GetTimerManager().ClearTimer(StageTimerHandle);
		ProjectileState.InitialLocation = FVector();
		AdvanceFlightStage();
	}
}

void AProjectile_Base::HandleTransition_RangeToTarget()
{
	if (!ProjectileMovementComponent->HomingTargetComponent.Get())
	{
		return;		//do whatever missed target
	}
	FVector TargetLocation = ProjectileMovementComponent->HomingTargetComponent->GetComponentLocation();
	float Distance = FVector::Dist(GetActorLocation(), TargetLocation);
	if (Distance <= ProjectileData->ProjectileFlightPlan[ProjectileState.FlightStageIndex].RequiredValue)
	{
		GetWorld()->GetTimerManager().ClearTimer(StageTimerHandle);
		AdvanceFlightStage();
	}
}

void AProjectile_Base::HandleTransition_Proximity2D()
{
	FVector TargetLocation = ProjectileMovementComponent->HomingTargetComponent->GetComponentLocation();
	float DistanceXY = FVector::Dist2D(GetActorLocation(), TargetLocation);
	if (DistanceXY <= ProjectileData->ProjectileFlightPlan[ProjectileState.FlightStageIndex].RequiredValue)		//if directly above
	{
		GetWorld()->GetTimerManager().ClearTimer(StageTimerHandle);
		AdvanceFlightStage();
	}
}

#pragma endregion

#pragma region Manual/GPSHoming

FVector AProjectile_Base::CalculateHomingToPoint(FVector TargetPoint)
{
	//home/guide to a POINT (no actor/component needed)
	//useful wire guided or designated point type projectiles
	return (TargetPoint - GetActorLocation()).GetSafeNormal() * ProjectileMovementComponent->HomingAccelerationMagnitude;
}

void AProjectile_Base::UpdateHomingPoint(FVector HomingPoint)
{
	ProjectileState.HomingTargetPoint = HomingPoint;
}

void AProjectile_Base::UpdateManualHoming(FVector TargetPoint)
{
	UpdateHomingPoint(TargetPoint);
	FVector DesiredDirection = (ProjectileState.HomingTargetPoint - GetActorLocation()).GetSafeNormal();
	FVector DeltaVelocity = CalculateHomingToPoint(TargetPoint);
	ProjectileMovementComponent->Velocity += DeltaVelocity * GetWorld()->GetDeltaSeconds();		//delta velocity
	ProjectileMovementComponent->Velocity = ProjectileMovementComponent->Velocity.GetSafeNormal() * ProjectileMovementComponent->MaxSpeed;
	SetActorRotation(ProjectileMovementComponent->Velocity.Rotation());
}

void AProjectile_Base::UpdateManualHoming_GPS()
{
	UpdateManualHoming(ProjectileState.HomingTargetPoint);
}

void AProjectile_Base::StartGPSGuidance()
{
	GetWorldTimerManager().SetTimer(ManualGuidanceTimerHandle, this, &AProjectile_Base::UpdateManualHoming_GPS, GetWorld()->GetDeltaSeconds(), true);
}

#pragma endregion

void AProjectile_Base::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	UNiagaraSystem* SelectedVFX = ProjectileData->ProjectileVisualData.ImpactVFX.LoadSynchronous();		//move this somewhere before impact so has time to load?
	if (SelectedVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), SelectedVFX, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
	}

	if (OtherActor && OtherActor != this)
	{
		// Extract the display name of the actor we collided with
		FString HitActorName = OtherActor->GetActorNameOrLabel();

		// Method A: Print directly to the Output Log Window
		UE_LOG(LogTemp, Log, TEXT("[Projectile Hit] Collided with: %s"), *HitActorName);

		// Method B: Print visually on the player's screen (Glows Cyan for 5 seconds)
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				5.0f,
				FColor::Cyan,
				FString::Printf(TEXT("Projectile Hit Target: %s"), *HitActorName)
			);
		}

		// --- Handle damage, explosion, or pooling return logic here ---
	}

	ProjectileMeshComponent->ClearMoveIgnoreActors();
	ProjectileMeshComponent->OnComponentHit.RemoveDynamic(this, &AProjectile_Base::OnHit);
	if (GetWorld()->GetTimerManager().IsTimerActive(ManualGuidanceTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(ManualGuidanceTimerHandle);
	}
	ProjectileState.FlightStageIndex = 0;
	ProjectileMovementComponent->Deactivate();
	NiagaraComponent->Deactivate();
	UBS2FunctionLibrary::GetProjectileSystem(this)->ReturnProjectileToPool(this);
}

void AProjectile_Base::OnImpact()
{
}
