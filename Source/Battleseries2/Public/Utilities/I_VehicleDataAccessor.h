#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "I_VehicleDataAccessor.generated.h"

struct FVehicleData;
struct FVehicleCurrentState;
class AVehicle_Base;

//RENAME!!!!!!!!!!!!!!!!!!!!!!

UINTERFACE(MinimalAPI)
class UVehicleDataAccessor : public UInterface
{
	GENERATED_BODY()
};

class BATTLESERIES2_API IVehicleDataAccessor
{
	GENERATED_BODY()

public:
	// Returns the main mesh component used for socket lookups (turrets, weapons, projectiles).
	virtual USkeletalMeshComponent* GetMesh() const = 0;
	virtual FName GetVehicleID() const = 0;
	virtual const FVehicleData& GetVehicleData() const = 0;
	virtual const FVehicleCurrentState& GetVehicleState() const = 0;
	virtual AVehicle_Base& GetVehicle() = 0;

};