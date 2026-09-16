#include "Core/UI/GameplayHUDs/UW_HUD_Status_Base.h"
#include "Components/TextBlock.h"
#include "Data/Items/Weapons/WeaponEnums.h"

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

void UUW_HUD_Status_Base::UpdateCurrentFireMode(EFireMode FireMode)
{
	//called on toggle fire mode
	TArray<UTextBlock*> TextBlocks = { T_Auto, T_Burst, T_Single };
	for (int32 i = 0; i < TextBlocks.Num(); i++)
	{
		UTextBlock* TextBlock = TextBlocks[i];
		if (TextBlock->GetRenderOpacity() == 0.0f)
		{
			TextBlocks.RemoveAt(i);
			continue;
		}
		TextBlock->SetRenderOpacity(FireModeNonActiveOpacity);
	}
	
	switch (FireMode)
	{
		case EFireMode::Auto:
			T_Auto->SetRenderOpacity(1.0f);
			break;
		case EFireMode::Burst:
			T_Burst->SetRenderOpacity(1.0f);
			break;
		case EFireMode::Single:
			T_Single->SetRenderOpacity(1.0f);
			break;
	}
}

void UUW_HUD_Status_Base::UpdateCanFireModes(TArray<bool> canFireModes)
{
	//called on equip weapon, partially lights up fire modes that can be used
	for (int32 i = 0; i < canFireModes.Num(); i++)
	{
		float OpacityValue = 0.0f;
		if (canFireModes[i])
		{
			OpacityValue = FireModeNonActiveOpacity;
		}
		
		switch (i)
		{
			case 0:
				T_Auto->SetRenderOpacity(OpacityValue);
				break;
			case 1:
				T_Burst->SetRenderOpacity(OpacityValue);
				break;
			case 2:
				T_Single->SetRenderOpacity(OpacityValue);
				break;
		}
	}
}
