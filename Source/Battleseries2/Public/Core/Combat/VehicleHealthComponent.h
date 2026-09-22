// coding in UE almost makes me wish for a nuclear winter

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/Core/Combat/CombatTypes.h"
#include "Utilities/I_VehicleDataAccessor.h"
#include "VehicleHealthComponent.generated.h"

/**
 *vehicle health component
 **/


//UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
UCLASS()
class BATTLESERIES2_API UVehicleHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVehicleHealthComponent();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FVehicleHealthState VehicleHealthState = FVehicleHealthState();
	
	UPROPERTY(BlueprintAssignable, Category = "Event Dispatchers")
	FOnHealthChanged OnVehicleHealthChanged;
	
	UFUNCTION(BlueprintCallable)
	void Init_VehicleHealth(float StartingHealth);
	
	UFUNCTION(BlueprintCallable)
	void HandleVehicleDestroyed();
	UFUNCTION(BlueprintCallable)
	void RevealDestroyedVehicle();
	
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Battleseries | Vehicles")
	FName GetVehicleArmorZone(const FVector& HitLocation);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);
	
private:
	IVehicleDataAccessor* OwnerDataAccessor;
	
	int32 CurrentLODStep = 0;
	FTimerHandle LODTimerHandle;
};