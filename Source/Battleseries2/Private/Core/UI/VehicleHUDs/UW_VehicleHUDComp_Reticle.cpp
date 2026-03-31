#include "Core/UI/VehicleHUDs/UW_VehicleHUDComp_Reticle.h"
#include "Components/Image.h"
#include "Components/ScaleBox.h"

void UUW_VehicleHUDComp_Reticle::NativeConstruct()
{
	Super::NativeConstruct();
}

void UUW_VehicleHUDComp_Reticle::UpdateReticleImage(UTexture2D* ImageBrush)
{
	Reticle->SetBrushFromTexture(ImageBrush);
}

void UUW_VehicleHUDComp_Reticle::UpdateReticleScale(float NewScale)
{
	ScaleBox_Reticle->SetUserSpecifiedScale(NewScale);
}

void UUW_VehicleHUDComp_Reticle::UpdateReticlePosition(FVector2D NewPosition)
{
	Reticle->SetRenderTranslation(NewPosition);
}

