#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Data/Vehicles/Data_Vehicle.h"
#include "Data/Core/CoreTypes.h"
#include "PlayerSave_Loadout.generated.h"

/**
* SaveGame class storing players choices for vehicles, weapons, etc
*/
UCLASS(Blueprintable, BlueprintType)
class BATTLESERIES2_API UPlayerSave_Loadout : public USaveGame
{
	GENERATED_BODY()

	public:
		//WeaponID-> player saved attachments
		UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TMap<FName, FPlayerLoadoutConfig_Weapon> WeaponConfigs;

		UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TMap<EClassType, FPlayerLoadoutConfig_Class> ClassLoadoutConfigs;

		/** Map: Vehicle category -> player's saved config */
		UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TMap<EVehicleType, FPlayerLoadoutConfig_Vehicle> VehicleLoadoutConfigs;
};
