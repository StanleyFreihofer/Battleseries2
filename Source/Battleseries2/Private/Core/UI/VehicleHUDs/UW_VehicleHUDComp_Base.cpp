#include "Core/UI/VehicleHUDs/UW_VehicleHUDComp_Base.h"

void UUW_VehicleHUDComp_Base::UpdateHUDColor_Implementation(FLinearColor NewColor)
{
	SetColorAndOpacity(NewColor);
}
