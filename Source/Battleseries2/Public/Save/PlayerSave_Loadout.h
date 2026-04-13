#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Data/Vehicles/Data_Vehicle.h"
#include "Data/Core/CoreTypes.h"
#include "PlayerSave_Loadout.generated.h"

/**
* SaveGame class storing players choices for vehicles, weapons, etc
*/
UCLASS()
class BATTLESERIES2_API UPlayerSave_Loadout : public USaveGame
{
	GENERATED_BODY()

	public:
		/** Map: Vehicle category -> player's saved config */
		UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TMap<E_VehicleType, FPlayerLoadoutConfig_Vehicle> VehicleConfigs;
};
