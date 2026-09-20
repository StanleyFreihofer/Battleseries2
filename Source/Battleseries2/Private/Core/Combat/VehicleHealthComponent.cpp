
#include "Core/Combat/VehicleHealthComponent.h"
#include "Utilities/BS2FunctionLibrary.h"
#include "Utilities/DataManagerSubsystem.h"
#include "Data/Vehicles/VehicleDefaults.h"
#include "GameFramework/Actor.h"

UVehicleHealthComponent::UVehicleHealthComponent()
{
}

void UVehicleHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	
	GetOwner()->OnTakeAnyDamage.AddDynamic(this, &UVehicleHealthComponent::HandleTakeAnyDamage);
}

void UVehicleHealthComponent::Init_VehicleHealth(float StartingHealth)
{
	VehicleHealthState.BaseHealthState.CurrentHealth = StartingHealth;
}

void UVehicleHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	float& CurrentHealth = VehicleHealthState.BaseHealthState.CurrentHealth;
	bool HealthDepleted = UBS2FunctionLibrary::TakeDmg(Damage, CurrentHealth);
	UE_LOG(LogTemp, Warning, TEXT("[VHC::HandleTakeAnyDamage] CurrentHealth = %f, Damage = %f"), CurrentHealth, Damage);
}



FName UVehicleHealthComponent::GetVehicleArmorZone(const FVector& HitLocation)
{
	//SOCKETS SHOULD BE PARENTED TO ROUT
	const AActor* Owner = GetOwner();
	USkeletalMeshComponent* VehicleMeshComponent = Cast<USkeletalMeshComponent>(Owner->GetRootComponent());
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
	if (AngleFromUp <= Defaults.VehicleCombatDefinition.TopArmorAngle)
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
