
#include "Core/Combat/VehicleHealthComponent.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Utilities/BS2FunctionLibrary.h"
#include "Utilities/DataManagerSubsystem.h"
#include "Data/Vehicles/VehicleDefaults.h"
#include "GameFramework/Actor.h"

class UNiagaraSystem;

UVehicleHealthComponent::UVehicleHealthComponent()
{
}

void UVehicleHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerDataAccessor = Cast<IVehicleDataAccessor>(GetOwner());
	GetOwner()->OnTakeAnyDamage.AddDynamic(this, &UVehicleHealthComponent::HandleTakeAnyDamage);
}

void UVehicleHealthComponent::Init_VehicleHealth(float StartingHealth)
{
	VehicleHealthState.BaseHealthState.CurrentHealth = StartingHealth;
}

void UVehicleHealthComponent::HandleVehicleDestroyed()
{
	TObjectPtr<USkeletalMeshComponent> VehicleMeshComponent = OwnerDataAccessor->GetMesh();
	TObjectPtr<UStaticMesh> DestroyedMesh = UBS2FunctionLibrary::GetDataSubsystem(GetOwner())->GetVehicleDataRow(OwnerDataAccessor->GetVehicleID())->DestroyedMesh.LoadSynchronous();
	UMaterialInterface* DestroyedMaterial = DestroyedMesh->GetMaterial(0);   
	VehicleHealthState.DestroyedMesh = NewObject<UStaticMeshComponent>(this);
	VehicleHealthState.DestroyedMesh->SetupAttachment(GetOwner()->GetRootComponent());
	FVector RelLoc = VehicleHealthState.DestroyedMesh->GetRelativeLocation();
	VehicleHealthState.DestroyedMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	VehicleHealthState.DestroyedMesh->SetVisibility(false);
	VehicleHealthState.DestroyedMesh->SetStaticMesh(DestroyedMesh);
	VehicleHealthState.DestroyedMesh->RegisterComponent();

	TArray<TSoftObjectPtr<UNiagaraSystem>> DestroyFXs = UBS2FunctionLibrary::GetDataSubsystem(this)->GetVehicleDefaults()->VehicleTypeDefintions.Find(OwnerDataAccessor->GetVehicleData().Vehicle_Type)->VehicleCombatDefinition.InitialFireballFX;
	int32 RandomIndex = FMath::RandRange(0, DestroyFXs.Num() - 1);
	TSoftObjectPtr<UNiagaraSystem> DestroyFX = DestroyFXs[RandomIndex];
	TObjectPtr<UNiagaraSystem> Explosion = DestroyFX.LoadSynchronous();
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), Explosion, GetOwner()->GetActorLocation(), GetOwner()->GetActorRotation());
	
	VehicleMeshComponent->SetMaterial(0, DestroyedMaterial);

    GetWorld()->GetTimerManager().SetTimer(VehicleHealthState.BaseHealthState.RegenTimer, this, &UVehicleHealthComponent::ShiftToDestroyedMesh, GetWorld()->GetDeltaSeconds(), true);
}

void UVehicleHealthComponent::ShiftToDestroyedMesh()
{
	TObjectPtr<USkeletalMeshComponent> VehicleMeshComponent = OwnerDataAccessor->GetMesh();
	const int32 NumBones = VehicleMeshComponent->GetNumBones();
	const int32 NumLODs = VehicleMeshComponent->GetSkeletalMeshAsset()->GetLODNum();
	int32 RandNumOfBonesToHide  = FMath::RandRange(10, NumBones);
	CurrentLODStep++;

	if (CurrentLODStep >= NumLODs)
	{
		RevealDestroyedVehicle();
		return;
	}
	
	for (int32 i = 0; i < RandNumOfBonesToHide; i++)
	{
		int32 RandBoneToHide = FMath::RandRange(0, VehicleMeshComponent->GetNumBones() - 1);
		FName BoneName = VehicleMeshComponent->GetBoneName(RandBoneToHide);
		VehicleMeshComponent->HideBoneByName(BoneName, PBO_None);
	}

	VehicleMeshComponent->SetForcedLOD(CurrentLODStep + 1);
}

void UVehicleHealthComponent::RevealDestroyedVehicle()
{
	TObjectPtr<USkeletalMeshComponent> VehicleMeshComponent = OwnerDataAccessor->GetMesh();
	GetWorld()->GetTimerManager().ClearTimer(VehicleHealthState.BaseHealthState.RegenTimer);
	VehicleMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	VehicleHealthState.DestroyedMesh->SetVisibility(true);
	VehicleHealthState.DestroyedMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	VehicleHealthState.DestroyedMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	VehicleHealthState.DestroyedMesh->SetSimulatePhysics(true);
	VehicleHealthState.DestroyedMesh->AddImpulse(FVector::UpVector * 500.f, NAME_None, true);
	VehicleMeshComponent->SetVisibility(false);
	
	TArray<FName> BoneNames; 
	VehicleMeshComponent->GetBoneNames(BoneNames);
	TArray<TSoftObjectPtr<UNiagaraSystem>> FireFXs = UBS2FunctionLibrary::GetDataSubsystem(this)->GetVehicleDefaults()->FireFX;
	for (int32 i = 0; i < FMath::RandRange(0, BoneNames.Num() - 1); i++)
	{
		int32 RandBoneIdx = FMath::RandRange(0, BoneNames.Num() - 1);
		FName RandBoneName = BoneNames[RandBoneIdx];
		int32 RandFXIdx = FMath::RandRange(0, FireFXs.Num() - 1);
		TObjectPtr<UNiagaraSystem> FireFX = FireFXs[RandFXIdx].LoadSynchronous();
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), FireFX, VehicleMeshComponent->GetBoneLocation(RandBoneName));
	}
}

void UVehicleHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	float& CurrentHealth = VehicleHealthState.BaseHealthState.CurrentHealth;
	bool HealthDepleted = UBS2FunctionLibrary::TakeDmg(Damage, CurrentHealth);
	OnVehicleHealthChanged.Broadcast();
	UE_LOG(LogTemp, Warning, TEXT("[VHC::HandleTakeAnyDamage] CurrentHealth = %f, Damage = %f"), CurrentHealth, Damage);
	
	if (HealthDepleted)
	{
		HandleVehicleDestroyed();
	}
}

FName UVehicleHealthComponent::GetVehicleArmorZone(const FVector& HitLocation)
{
	//SOCKETS SHOULD BE PARENTED TO ROOT
	const AActor* Owner = GetOwner();
	USkeletalMeshComponent* VehicleMeshComponent = OwnerDataAccessor->GetMesh();
	const UDA_VehicleDefaults& Defaults = *UBS2FunctionLibrary::GetDataSubsystem(GetOwner())->GetVehicleDefaults();
	const FVector ToHit = (HitLocation - GetOwner()->GetActorLocation()).GetSafeNormal();
	
	const FVector FrontLoc = VehicleMeshComponent->GetSocketLocation("HullFront");
	const FVector RearLoc  = VehicleMeshComponent->GetSocketLocation("HullRear");
	float HullHalfLength = FVector::Dist(FrontLoc, RearLoc) * 0.5f;

	const FVector LeftLoc  = VehicleMeshComponent->GetSocketLocation("HullLeft");
	const FVector RightLoc = VehicleMeshComponent->GetSocketLocation("HullRight");
	float HullHalfWidth = FVector::Dist(LeftLoc, RightLoc) * 0.5f;
	
	// Top stays angle-based — a cone around "up" is the right shape for this zone
	const float AngleFromUp = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Owner->GetActorUpVector(), ToHit)));
	if (AngleFromUp <= Defaults.VehicleTypeDefintions.Find(OwnerDataAccessor->GetVehicleData().Vehicle_Type)->VehicleCombatDefinition.TopArmorAngle)
	{
		return "TopArmor";
	}

	// Front/Rear/Side: project into vehicle-local space, normalize by the
	// vehicle's real half-extents, let the dominant axis decide the zone.
	const FVector LocalDir = Owner->GetActorTransform().InverseTransformVectorNoScale(ToHit);

	const float NormForward = LocalDir.X / HullHalfLength; // +X = front, -X = rear
	const float NormRight   = LocalDir.Y / HullHalfWidth;  // +Y = right, -Y = left

	if (FMath::Abs(NormForward) >= FMath::Abs(NormRight))
	{
		return NormForward >= 0.f ? "FrontArmor" : "RearArmor";
	}
	return "SideArmor";
}
