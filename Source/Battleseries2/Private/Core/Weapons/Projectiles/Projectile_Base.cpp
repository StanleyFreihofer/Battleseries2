#include "Core/Weapons/Projectiles/Projectile_Base.h"
#include "Data/Weapons/Data_Projectile.h"
#include "Utilities/DataManagerSubsystem.h"
#include "Utilities/GameInstance_Base.h"
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
	UDataManagerSubsystem* DataManager = GetDataManager();
	const FProjectileData* ProjectileRow = DataManager->GetProjectileDataRow(ProjectileState.MunitionID);
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
	Init_ProjectileFlightData();
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

void AProjectile_Base::Init_ProjectileFlightData()
{
	//NEEDED?
	/**
	switch (ProjectileData->ProjectileFlightPlan[0].BehaviorType)
	{
		case EProjectileGuidanceMethod::BallisticTrajectory:
			ProjectileMovementComponent->InitialSpeed = ProjectileState.CurrentVelocity.Size();
			ProjectileMovementComponent->MaxSpeed = ProjectileData->ProjectileFlightPlan[0].GuidanceParams.MaxSpeed;
			break;
	}
	**/
}

void AProjectile_Base::SetRuntimeContext(UPrimitiveComponent* AttachComponent, FName AttachSocket)
{
	//PREFLIGHT CONTEXT
	if (AttachComponent)
	{
		this->AttachToComponent(AttachComponent, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), AttachSocket);
		ProjectileState.PreFlightContext.AttachedComponent = AttachComponent;
		ProjectileState.PreFlightContext.AttachSocket = AttachSocket;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Projectile_Base::SetRuntimeContext] AttachComponent invalid, should detach happen here?"));
		ProjectileState.PreFlightContext.AttachedComponent = nullptr;
		ProjectileState.PreFlightContext.AttachSocket = NAME_None;
	}
}

void AProjectile_Base::FireProjectile(FVector AimDirection)
{
	if (ProjectileState.PreFlightContext.AttachedComponent || GetParentComponent() != nullptr)
	{
		//FVector VehicleVelocity = GetAttachParentActor()->GetVelocity();
		UE_LOG(LogTemp, Warning, TEXT("[ProjectilePoolSubsystem::ReturnProjectileToPool] DetachFromActor"));
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		//add impulse or something idk
	}

	ProjectileState.AimDirection = AimDirection;

	StartFlightPlan();
	UpdateFlightPlan(0);
}

void AProjectile_Base::StartFlightPlan()
{
	ProjectileMeshComponent->OnComponentHit.AddDynamic(this, &AProjectile_Base::OnHit);		//should not explode if not in use yet (hanging on rack for example)
	ProjectileMovementComponent->Velocity = ProjectileState.AimDirection * ProjectileData->ProjectileFlightPlan[0].GuidanceParams.InitialSpeed;
	ProjectileMovementComponent->Activate();
	NiagaraComponent->Activate();
}

void AProjectile_Base::UpdateFlightPlan(int32 FlightStageIndex)
{
	const FProjectileFlightStage& FlightStage = ProjectileData->ProjectileFlightPlan[FlightStageIndex];

	//undo stuff from previous stage

	//guidance params
	ProjectileMovementComponent->InitialSpeed = FlightStage.GuidanceParams.InitialSpeed;
	ProjectileMovementComponent->MaxSpeed = FlightStage.GuidanceParams.MaxSpeed;
	ProjectileMovementComponent->ProjectileGravityScale = FlightStage.GuidanceParams.GravityScale;

	//behavior
	switch (FlightStage.BehaviorType)
	{
		case EProjectileGuidanceMethod::BallisticTrajectory:
			ProjectileMovementComponent->Velocity = ProjectileState.AimDirection * FlightStage.GuidanceParams.InitialSpeed;
			break;
		case EProjectileGuidanceMethod::WireGuided:	//guides to component at end of hit result
		case EProjectileGuidanceMethod::GuideToTarget:
			ProjectileMovementComponent->bIsHomingProjectile = true;
			ProjectileMovementComponent->HomingAccelerationMagnitude = FlightStage.GuidanceParams.Acceleration;
			break;
		case EProjectileGuidanceMethod::PitchToAltitude:
			FVector UpwardPitch = FVector::UpVector * FlightStage.GuidanceParams.PitchForce;
			ProjectileMovementComponent->Velocity += UpwardPitch;
			break;
	}
	HandleFlightStageTransition(FlightStage);
}

void AProjectile_Base::HandleFlightStageTransition(const FProjectileFlightStage& FlightStage)
{
	switch (FlightStage.PrimaryTransitionCondition)
	{
		case ETransitionCondition::LimitedTime:
			GetWorld()->GetTimerManager().SetTimer(StageTimerHandle, this, &AProjectile_Base::AdvanceFlightStage, FlightStage.RequiredValue, false);
			break;
		case ETransitionCondition::LimitedRange:
			ProjectileState.OriginLocation = GetActorLocation();
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
	float Distance = FVector::Dist(ProjectileState.OriginLocation, EndPoint);
	if (Distance >= ProjectileData->ProjectileFlightPlan[ProjectileState.FlightStageIndex].RequiredValue)
	{
		GetWorld()->GetTimerManager().ClearTimer(StageTimerHandle);
		ProjectileState.OriginLocation = FVector();
		AdvanceFlightStage();
	}
}

void AProjectile_Base::HandleTransition_RangeToTarget()
{
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

void AProjectile_Base::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	UNiagaraSystem* SelectedVFX = ProjectileData->ProjectileVisualData.ImpactVFX.LoadSynchronous();
	if (SelectedVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), SelectedVFX, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
	}
	ProjectileMeshComponent->ClearMoveIgnoreActors();
	ProjectileMeshComponent->OnComponentHit.RemoveDynamic(this, &AProjectile_Base::OnHit);
	ProjectileState.FlightStageIndex = 0;
	ProjectileMovementComponent->Deactivate();
	NiagaraComponent->Deactivate();
	GetProjectileSystem()->ReturnProjectileToPool(this);
}

void AProjectile_Base::OnImpact()
{
}

UDataManagerSubsystem* AProjectile_Base::GetDataManager()
{
	return GetGameInstance()->GetSubsystem<UDataManagerSubsystem>();
}

UProjectilePoolSubsystem* AProjectile_Base::GetProjectileSystem()
{
	return GetWorld()->GetSubsystem<UProjectilePoolSubsystem>();
}

