#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Data/Weapons/Data_Weapon.h"
#include "Data_VehicleWeapon.generated.h"

//list of ALL VEHICLE WEAPONS
//vehicle weapon specific stuff

//REMEMBER, ID IS THE DT ROW NAME

USTRUCT(BlueprintType)
struct FVehicleWeaponData : public FTableRowBase
{
	GENERATED_BODY()

	//base weapon data (from Data_Weapon) TO BE FILLED IN HERE
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBaseWeaponData WeaponData;
};

