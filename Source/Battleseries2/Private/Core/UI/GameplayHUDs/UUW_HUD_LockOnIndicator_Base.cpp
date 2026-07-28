#include "Core/UI/GameplayHUDs/UUW_HUD_LockOnIndicator_Base.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Overlay.h"
#include "Components/CanvasPanelSlot.h" 
#include "Data/Items/Weapons/WeaponEnums.h"

void UUW_HUD_LockOnIndicator_Base::UpdateIndicatorPosition(FVector Location)
{
    APlayerController* PlayerController = GetOwningPlayer();
    UCanvasPanelSlot* IndicatorSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(IndicatorGroup);
    FVector2D IndicatorScreenPosition; 
    UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PlayerController, Location, IndicatorScreenPosition, false);

    IndicatorSlot->SetPosition(IndicatorScreenPosition);
}

void UUW_HUD_LockOnIndicator_Base::UpdateLockIndicatorStatus(ELockOnState LockOnState)
{
    switch (LockOnState)
    {
        case ELockOnState::NotLockingOn:
            break;
        case ELockOnState::IsLockingOn:
            break;
        case ELockOnState::IsLockedOn:
            Diamond->SetVisibility(ESlateVisibility::Visible);
            break;
        case ELockOnState::IsLosingLock:
            Diamond->SetVisibility(ESlateVisibility::Hidden);
            break;
    }
}