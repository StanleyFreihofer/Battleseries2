#include "Core/UI/GameplayHUDs/UW_HUD_Status_Base.h"
#include "Components/TextBlock.h"

void UUW_HUD_Status_Base::UpdateCAMCount(int32 CAM)
{
	FString FormattedAmmo = FString::Printf(TEXT("%03d"), CAM);
	T_CurrentAmmoInMag->SetText(FText::FromString(FormattedAmmo));
}

void UUW_HUD_Status_Base::UpdateCRACount(int32 CRA)
{
	FString FormattedAmmo = FString::Printf(TEXT("%03d"), CRA);
	T_CurrentReserveAmmo->SetText(FText::FromString(FormattedAmmo));
}
