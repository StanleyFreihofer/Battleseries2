#include "Core/UI/VehicleHUDs/UW_VehicleHUDComp_AltitudeGauge.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"

void UUW_VehicleHUDComp_AltitudeGauge::UpdateAltitudeGaugePosition(float Pitch)
{
	float Position = Pitch * -1.0f * 4.6f;
	UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Img_AltitudeGauge->Slot);
	float CurrentPosition = CanvasSlot->GetPosition().Y;
	float NewPosition = Position + CurrentPosition;
	CanvasSlot->SetPosition(FVector2D(CanvasSlot->GetPosition().X, Position));
}
