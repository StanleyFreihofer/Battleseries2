#include "Core/UI/VehicleHUDs/UW_VehicleHUDComp_Optic.h"
#include "Components/TextBlock.h"

void UUW_VehicleHUDComp_Optic::UpdateOpticName(FText OpticName)
{
	T_OpticName->SetText(OpticName);
}

void UUW_VehicleHUDComp_Optic::UpdateOpticMagnification(float OpticMagnification)
{
	int32 NewOpticMagnif = FMath::RoundToInt(OpticMagnification);
	T_OpticMagnification->SetText(FText::Format(FText::FromString("{0}x"), FText::AsNumber(NewOpticMagnif)));
}
