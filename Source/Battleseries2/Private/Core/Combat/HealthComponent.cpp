// coding in UE almost makes me wish for a nuclear winter


#include "Core/Combat/HealthComponent.h"
#include "Character_Base.h"
#include "Utilities/BS2FunctionLibrary.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Actor.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	
	GetOwner()->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::HandleTakeAnyDamage);

	// ...
}

// Called every frame
void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UHealthComponent::Init_Health(float StartingHealth)
{
	HealthState.CurrentHealth = StartingHealth;
}

void UHealthComponent::KillCharacter()
{
	USkeletalMeshComponent* TPMesh = GetOwnerCharacter()->GetMesh();
	UCapsuleComponent* Capsule = GetOwnerCharacter()->GetCapsuleComponent();
	
	TPMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	TPMesh->SetSimulatePhysics(true);
	Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void UHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	float& CurrentHealth = HealthState.CurrentHealth;
	bool HealthDepleted = UBS2FunctionLibrary::TakeDmg(Damage, CurrentHealth);
	if (HealthDepleted)
	{
		KillCharacter();
	}
}

ACharacter_Base* UHealthComponent::GetOwnerCharacter()
{
	return Cast<ACharacter_Base>(GetOwner());
}

