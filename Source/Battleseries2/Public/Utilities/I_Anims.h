#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
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
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Anim Interface")
	void OnUpdateTurret(float Rotation, float Pitch);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Anim Interface")
	void OnFireWeapon_Vehicle(int32 SeatIndex, int32 WeaponIndex);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Anim Interface")
	void OnEnterSeat_Vehicle(int32 SeatIndex);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Anim Interface")
	void OnUpdateHeadRotation(FRotator NewRotation);

};