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

void USpawnComponent::Init_SpawnData(FSpawnData_Runtime InputData)
{
	SpawnData = InputData;
}
