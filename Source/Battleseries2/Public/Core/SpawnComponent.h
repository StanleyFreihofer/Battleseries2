#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "SpawnComponent.generated.h"

UENUM(BlueprintType)
enum class ESpawnType : uint8
{
	Flag,
	Squadmate,
	Vehicle
	//radio beacon, teammate, on and on
};

USTRUCT(BlueprintType)
struct FSpawnData_Runtime						//defines an entire weapon on the vehicle
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText SpawnDisplayName = FText();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> SpawnIcon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESpawnType SpawnType = ESpawnType::Flag;
};


UCLASS(Blueprintable, BlueprintType)
class BATTLESERIES2_API USpawnComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	USpawnComponent();
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSpawnData_Runtime SpawnData = FSpawnData_Runtime();

	UFUNCTION(BlueprintCallable)
	void Init_SpawnData(FSpawnData_Runtime InputData);
};