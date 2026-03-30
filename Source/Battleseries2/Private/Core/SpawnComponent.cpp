#include "Core/SpawnComponent.h"

USpawnComponent::USpawnComponent()
{
}


void USpawnComponent::BeginPlay()
{
	Super::BeginPlay();
}

void USpawnComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void USpawnComponent::Init_SpawnData(FText SpawnDisplayName, UTexture2D* SpawnIcon, ESpawnType SpawnType)
{
	SpawnData.SpawnDisplayName = SpawnDisplayName;
	SpawnData.SpawnIcon = SpawnIcon;
	SpawnData.SpawnType = SpawnType;
}
