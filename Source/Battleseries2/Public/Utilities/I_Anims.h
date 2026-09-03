#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Data/Items/Weapons/Data_InfantryWeapon.h"
#include "Data/Items/Gadgets/Data_Gadget.h"
#include "I_Anims.generated.h"

UINTERFACE(MinimalAPI)
class UAnims : public UInterface
{
	GENERATED_BODY()
};

class BATTLESERIES2_API IAnims
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Anim Interface | Vehicle")
	void OnUpdateTurret(float Rotation, float Pitch);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Anim Interface | Vehicle")
	void OnFireWeapon_Vehicle(int32 SeatIndex, int32 WeaponIndex);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Anim Interface | Vehicle ")
	void OnEnterSeat_Vehicle(int32 SeatIndex);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Anim Interface | Character")
	void OnUpdateHeadRotation(FRotator NewRotation);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Anim Interface | Character")
	void OnEquipWeapon_FP(FInfantryWeaponAnimData_FP InfantryWeaponAnimData_FP);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Anim Interface | Character")
	void OnEquipGadget(FGadgetAnimData GadgetAnimData);

};