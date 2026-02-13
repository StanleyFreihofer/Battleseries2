#include "Core/UI/GameplayHUDs/UUW_HUD_LockOnIndicator_Base.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanelSlot.h" 

void UUW_HUD_LockOnIndicator_Base::UpdateIndicatorPosition(FVector Location)
{
    UCanvasPanelSlot* IndicatorSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(LockOnIndicator);
    FVector2D IndicatorScreenPosition; 
    APlayerController* PlayerController = GetOwningPlayer();
    UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PlayerController, Location, IndicatorScreenPosition, false);

    IndicatorSlot->SetPosition(IndicatorScreenPosition);
}
