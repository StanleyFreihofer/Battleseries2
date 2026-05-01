#include "Core/UI/VehicleHUDs/UW_VehicleHUDComp_TurretBox.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"

void UUW_VehicleHUDComp_TurretBox::UpdateOrientationPosition(float Rotation, float Pitch)
{
	float InversePitch = Pitch * -1.0f;
	Img_OrientationMarker->SetRenderTranslation(FVector2D(Rotation, InversePitch));
}
